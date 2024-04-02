#pragma once

#include"__AudioCommon.h"

extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/timestamp.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

#pragma comment(lib, "../Third-Party/FFmpeg/lib/avcodec")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/avdevice")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/avfilter")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/avutil")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/avformat")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/postproc")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/swscale")
#pragma comment(lib, "../Third-Party/FFmpeg/lib/swresample")

#include<mutex>

namespace Suancai {

	namespace Media {

		namespace Audio {

			template<typename T>
			class CircularBuffer {
			public:
				enum Ret : i8 {
					Ok,
					KeyRegistered
				};
			public:
				i32 channels = 0; // * do not change this field except you are the member function
				i32 channelSamples = 0; // * do not change this field except you are the member function
				std::atomic<i32> logicalReadStart;
				std::atomic<i32> logicalWriteStart; // * not include the last data idx
				std::vector<std::vector<T>> buffer;
			public:
				CircularBuffer();
				void init(i32 channels, i32 channelSamples);
				
				i32 getLogicalReadStartIdx();

				i32 getLogicalWriteStartIdx();
				
				i32 getUsedSpaceCnt();
				i32 getFreeSpaceCnt();
				
				void forwardReadStart(i32 step);
				void forwardWriteStart(i32 step);
				//
				i32 getChannels();
				i32 getChannelSamples();
				~CircularBuffer();
			};

			class AudioDecoder {
			protected:
				AudioSampleFormat sampleFormat;
				i32 channels = 0;
				i32 sampleRate = 0;
				i32 duration = 0;
				std::pair<float, float> durationTimeBase;
				//
				AVFormatContext* pFmtCtx = nullptr;
				const AVCodec* pCdc = nullptr;
				AVCodecContext* pCdcCtx = nullptr;
				AVCodecParserContext* pCdcPasrCtx = nullptr;
				AVFrame* pFrame = nullptr;
				AVPacket* pPkt = nullptr;
				i32 aud_strm_idx = -1;
				AVStream* pAudStream = nullptr;
				//
				i32 dstSampleRate = 0;
				std::vector<float> tempDstBuffer; // cb buffer typically five times the size of end buffer
			public:
				CircularBuffer<float> decodeCBuffer;
			public:
				std::vector<float> allSamples;
				void openFile(char8_t* pPath, i32 decodeCBufferSize, i32 dstSampleRate);
				i32 decode(i32 minimumSamples);
				CircularBuffer<float>& getCBuffer();
				void resample(float* src, i32 srcSize, float* dst, i32 dstSize);
			};


			template<typename T>
			Suancai::Media::Audio::CircularBuffer<T>::CircularBuffer() {
			}

			template<typename T>
			void Suancai::Media::Audio::CircularBuffer<T>::init(i32 channels, i32 channelSamples) {

				this->channels = channels;
				this->channelSamples = channelSamples;
				this->buffer.assign(channels, std::vector<T>(channelSamples));
				this->logicalReadStart.store(0);
				this->logicalWriteStart.store(0);
			}
			template<typename T>
			inline i32 CircularBuffer<T>::getLogicalReadStartIdx() {
				return this->logicalReadStart.load();
			}
			template<typename T>
			inline i32 CircularBuffer<T>::getLogicalWriteStartIdx() {
				return this->logicalWriteStart.load();
			}
			template<typename T>
			inline i32 CircularBuffer<T>::getUsedSpaceCnt() {
				i32 read = this->logicalReadStart.load();
				i32 write = this->logicalWriteStart.load();
				return write - read;
			}
			template<typename T>
			inline i32 CircularBuffer<T>::getFreeSpaceCnt() {
				i32 read = this->logicalReadStart.load();
				i32 write = this->logicalWriteStart.load();
				return this->channelSamples - (write - read);
			}
			template<typename T>
			inline void CircularBuffer<T>::forwardReadStart(i32 step) {
				this->logicalReadStart.fetch_add(step);
			}
			template<typename T>
			inline void CircularBuffer<T>::forwardWriteStart(i32 step) {
				this->logicalWriteStart.fetch_add(step);
			}
			template<typename T>
			inline i32 CircularBuffer<T>::getChannels() {
				return this->channels;
			}
			template<typename T>
			inline i32 CircularBuffer<T>::getChannelSamples() {
				return this->channelSamples;
			}
			template<typename T>
			inline Suancai::Media::Audio::CircularBuffer<T>::~CircularBuffer() {
				
			}
		}
	}
}