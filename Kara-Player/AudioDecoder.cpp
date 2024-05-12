#include "AudioDecoder.h"

using namespace Suancai;

void Suancai::Media::Audio::AudioDecoder::openFile(char8_t* pPath, i32 decodeCBufferSize, i32 dstSampleRate) {
    
	this->clearOpenedResource();

	char* pFilePath = (char*)pPath;

	int ret = 0;

	// * open file
	pFmtCtx = avformat_alloc_context();
	CHECK_AND_THROW((pFmtCtx == nullptr), "cant alloc fmt ctx", -1);
	ret = avformat_open_input(addr(pFmtCtx), pFilePath, nullptr, nullptr);
	CHECK_AND_THROW((ret != 0), "无法打开avformatctx", -1);

	// * get audio stream idx
	for (int i = 0; i < pFmtCtx->nb_streams; i++) {
		if (pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			aud_strm_idx = i;
			pAudStream = pFmtCtx->streams[i];
			break;
		}
	}
	CHECK_AND_THROW((ret == -1), "cant get aud strm", -1);
	
	// * init channel data
	this->channels = this->pAudStream->codecpar->ch_layout.nb_channels;
	this->sampleRate = this->pAudStream->codecpar->sample_rate;
	this->duration = this->pAudStream->duration;
	this->durationTimeBase = std::pair<float, float>(this->pAudStream->time_base.num, this->pAudStream->time_base.den);
	this->decodeCBuffer.init(2, decodeCBufferSize);
	this->dstSampleRate = dstSampleRate;

	pCdc = avcodec_find_decoder(pAudStream->codecpar->codec_id);
	CHECK_AND_THROW((pCdc == nullptr), "cant find codec", -1);

	pCdcCtx = avcodec_alloc_context3(pCdc);
	CHECK_AND_THROW((pCdcCtx == nullptr), "cant alloc cdc ctx", -1);
	ret = avcodec_parameters_to_context(pCdcCtx, pAudStream->codecpar);
	CHECK_AND_THROW((ret != 0), "cant copy params", -1);
	ret = avcodec_open2(pCdcCtx, pCdc, nullptr);
	CHECK_AND_THROW((ret != 0), "cant open codec", -1);
	
	pCdcCtx->time_base = pAudStream->time_base;

	// * alloc pkts and frames, get ready to decode
	pPkt = av_packet_alloc();
	CHECK_AND_THROW((pPkt == nullptr), "cant alloc pkt", -1);
	pFrame = av_frame_alloc();
	CHECK_AND_THROW((pFrame == nullptr), "cant alloc frame", -1);
}

i32 Suancai::Media::Audio::AudioDecoder::decode(i32 minimumSamples) {

	int ret = 0;
	i32 samplesDecoded = 0;
	float dstResampleBufferSizeRatio = (float)this->dstSampleRate / this->sampleRate;
	i32 dstResampleBufferCnt = 0;
	i32 channelSamples = this->decodeCBuffer.getChannelSamples();
	// * 
    while (av_read_frame(pFmtCtx, pPkt) >= 0) {
        if (pPkt->stream_index != aud_strm_idx) {
            continue;
        }
        ret = avcodec_send_packet(pCdcCtx, pPkt);
        if (ret != 0 && ret != ret == AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            CHECK_AND_THROW((pFrame == nullptr), "cant send packet", -1);
        }
        while (true) {
            ret = avcodec_receive_frame(pCdcCtx, pFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
				// if not eagain or eof, something unexpected happened, throw exception
                CHECK_AND_THROW(TRUE, "cant recv packet", -1);
            }
			//float time = float(pFrame->pts * pAudStream->time_base.num) / pAudStream->time_base.den;
			//float duration = float(pFrame->duration * pAudStream->time_base.num) / pAudStream->time_base.den;
			if (pFrame->format == AVSampleFormat::AV_SAMPLE_FMT_FLTP) {
				// * ready to resample
				// * first check the free space
				dstResampleBufferSizeRatio = (float)this->dstSampleRate / pFrame->sample_rate;
				dstResampleBufferCnt = (i32)floor(dstResampleBufferSizeRatio * pFrame->nb_samples);
				while (this->decodeCBuffer.getFreeSpaceCnt() < dstResampleBufferCnt) {
					Sleep(0);
				}
				// * now we are guarenteed that there is enough space to write
				this->tempDstBuffer.resize(dstResampleBufferCnt);
				// * for each channel copy to tempSrcBuffer
				for (int chIdx = 0; chIdx < pFrame->ch_layout.nb_channels; chIdx++) {
					i32 startIdx = this->decodeCBuffer.getLogicalWriteStartIdx() % channelSamples;
					// resample to writeIdx
					this->resample((float*)pFrame->data[chIdx], pFrame->nb_samples, this->tempDstBuffer.data(), dstResampleBufferCnt);
					for (int sampleIdx = 0; sampleIdx < dstResampleBufferCnt; sampleIdx++) {
						this->decodeCBuffer.buffer[chIdx][(startIdx + sampleIdx) % channelSamples] = this->tempDstBuffer[sampleIdx];
					}
				}
				// * the forward
				this->decodeCBuffer.forwardWriteStart(dstResampleBufferCnt);
				// before return we check if we reach the minimum samples limit
				samplesDecoded += pFrame->nb_samples;
				if (samplesDecoded >= minimumSamples) {
					return samplesDecoded;
				}
				// else we dont return util we meet the limit or eof/error
			}
        }
    }
	// if not decode any data, we simply return -1, means error or eof
	// indicates that user should play some other file
	return -1;
}

Suancai::Media::Audio::CircularBuffer<float>& Suancai::Media::Audio::AudioDecoder::getCBuffer() {
	// TODO: insert return statement here
	return this->decodeCBuffer;
}

void Suancai::Media::Audio::AudioDecoder::resample(float* src, i32 srcSize, float* dst, i32 dstSize) {
	
	// * assume src and dst have the same channels
	float sCnt = srcSize;
	float dCnt = dstSize;
	for (i32 dSampleIdx = 0; dSampleIdx < dstSize; dSampleIdx++) {
		float dT = dSampleIdx / dCnt;
		// * transform sT to dT
		float d2sI = dT * (sCnt - 1);
		i32 l = floor(d2sI);
		i32 r = l + 1;
		float t = (d2sI - l) / (r - l);
		// * do interpolation
		dst[dSampleIdx] = (1.0f - t) * src[l] + t * src[r];
	}
}

void Suancai::Media::Audio::AudioDecoder::clearOpenedResource() {

	av_packet_free(&this->pPkt);
	av_frame_free(&this->pFrame);
	avcodec_free_context(&this->pCdcCtx);
	avformat_free_context(this->pFmtCtx);
}
