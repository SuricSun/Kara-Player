#include "KaraPlayer.h"

using namespace Suancai;
using namespace Suancai::Media::Audio;

unsigned __stdcall Suancai::Player::KaraPlayer::AudioThread(void* pArg) {

    try {
        KaraPlayer* p = (KaraPlayer*)pArg;
        u32 free = 0, total = 0;
        bool start = false;
        i32 fftSize = 8192;
        float* pWindowedL = new float[fftSize];
        float* pWindowedR = new float[fftSize];
        float* pAudioL = new float[fftSize];
        float* pAudioR = new float[fftSize];
        FFT fftL;
        FFT fftR;
        fftL.init(fftSize);
        fftR.init(fftSize);
        i32 readSampleLogicalStart = 0;
        // * init
        AudioEnumerator emu;
        emu.init();
        p->audioRenderer.init(emu.getDefaultRenderDevice(), 10000000, false);
        p->audioRenderer.start();
        p->vr.init();
        // * idle
        Message msg;
        while (true) {
            // * take message
            if (p->toAudioQ.pop(msg)) {
                switch (msg.type) {
                    case Message::Type::Play:
                    {
                        // * every time recv PLAY message, we set start to true
                        // * reset audio stream
                        p->audioRenderer.stop();
                        p->audioRenderer.reset();
                        p->audioRenderer.start();
                        start = true;
                        break;
                    }
                }
            }
            if (start == false) {
                continue;
            }
            // * do i need to refill audio?
            p->audioRenderer.getFreeSpaceCnt(free, total);
            i32 channelSamples = p->audioDecoder.decodeCBuffer.getChannelSamples();
            i32 logicalRead = p->audioDecoder.decodeCBuffer.getLogicalReadStartIdx();
            i32 logicalWrite = p->audioDecoder.decodeCBuffer.getLogicalWriteStartIdx();
            i32 availableSamples = p->audioDecoder.decodeCBuffer.getUsedSpaceCnt();
            if (free >= (total / 2)) {
                // * need to refill
                // * but do we have enough supplied data?
                // * nah we just put as much as we can
                u32 samplesToRead = min(free, logicalWrite - readSampleLogicalStart);
                float* pBuffer = (float*)p->audioRenderer.getFreeSpacePointer(samplesToRead);
                i32 channelCnt = 2;
                for (int sampleIdx = 0; sampleIdx < samplesToRead; sampleIdx++) {
                    for (int chIdx = 0; chIdx < channelCnt; chIdx++) {
                        // * send them to buffer, but interleaved
                        pBuffer[sampleIdx * channelCnt + chIdx] = p->audioDecoder.decodeCBuffer.buffer[chIdx][(readSampleLogicalStart + sampleIdx) % channelSamples];
                    }
                }
                readSampleLogicalStart += samplesToRead;
                // * after done£¬ release buffer and forward idx
                p->audioRenderer.releaseFreeSpacePointer(samplesToRead);
            }
            // * need to render
            auto& pos = p->audioRenderer.getDevicePosition();
            double time = double(pos.first) / pos.second;
            p->time = time;
            //OutputDebugStringA(to_string(time).append("\n").c_str());
            i32 desiredLogicalRead = (time) * 48000;
            p->sample = desiredLogicalRead;
            if (desiredLogicalRead < logicalRead) {
                desiredLogicalRead = logicalRead;
            }
            // * windowing
            pAudioL[0] = p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead) % channelSamples];
            pAudioR[0] = p->audioDecoder.decodeCBuffer.buffer[1][(desiredLogicalRead) % channelSamples];
            pWindowedL[0] = 0;
            pWindowedR[0] = 0;
            for (int i = 1; i < fftSize; i++) {
                pAudioL[i] = p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + i) % channelSamples];
                pAudioR[i] = p->audioDecoder.decodeCBuffer.buffer[1][(desiredLogicalRead + i) % channelSamples];
                pWindowedL[i] =
                    pow((0.5f - (1.0f - 0.5f) * cos((2.0f * PI_F * i) / float((fftSize - 1)))), 1) *
                    (pAudioL[i] - 0.93f * pAudioL[i - 1]);
                pWindowedR[i] =
                    pow((0.5f - (1.0f - 0.5f) * cos((2.0f * PI_F * i) / float((fftSize - 1)))), 1) *
                    (pAudioR[i] - 0.93f * pAudioR[i - 1]);
            }
            /*for (int i = 2; i < fftSize; i++) {
				pWindowed[i] =
					0.8f * p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + i) % channelSamples] +
					0.0f * p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + abs(i - 1)) % channelSamples] +
					0.8f * p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + abs(i - 2)) % channelSamples] +
					-2.4f * pWindowed[i - 1] +
					0.989f * pWindowed[i - 2];
                pWindowed[i] *= pow((0.5f - (1.0f - 0.5f) * cos((2.0f * PI_F * i) / float((fftSize - 1)))), 2);
			}*/
            p->audioDecoder.decodeCBuffer.forwardReadStart(desiredLogicalRead - logicalRead);
            fftL.doFFT(pWindowedL, fftSize, 0);
            fftR.doFFT(pWindowedR, fftSize, 0);
            p->vr.clear();
            p->vr.setAlphaBlend(false);
            p->vr.render(pAudioL, pAudioR, fftL.p_result1_final, fftSize / 2, true);
            p->vr.render(pAudioL, pAudioR, fftR.p_result1_final, fftSize / 2, false);
            p->vr.blurAndPresent();
        }
    } catch (Suancai::Exception::BaseException* pE) {
        pE->showMsg();
        del(pE);
    }

    return 0;
}

unsigned __stdcall Suancai::Player::KaraPlayer::DecodeThread(void* pArg) {

    KaraPlayer* p = (KaraPlayer*)pArg;
    bool start = false;
    try {
        KaraPlayer* p = (KaraPlayer*)pArg;
        u32 free = 0, total = 0;
        // * idle
        Message msg;
        while (true) {
            // * take message
            if (p->toDecodeQ.pop(msg)) {
                switch (msg.type) {
                    case Message::Type::Play:
                    {
                        // * now open new file
                        p->audioDecoder.openFile((char8_t*)msg.strData.c_str(), 48000 * 10 * 4, 48000);
                        start = true;
                        p->toAudioQ.push(Message{Message::Play, 0, 0, nullptr});
                        break;
                    }
                }
            }
            if (!start) {
                continue;
            }
            // * now decode if ASS WE CAN
            // * decode the whole end point buffer size
            p->audioDecoder.decode(48000);
            // * then we sleep for half that length
            //Sleep((10.0f / 2.0f) * 1000);
        }
    } catch (Suancai::Exception::BaseException* pE) {
        pE->showMsg();
        del(pE);
    }
    
    return 0;
}

Suancai::Player::KaraPlayer::KaraPlayer() {

    _beginthreadex(0, 0, KaraPlayer::DecodeThread, this, 0, 0);
    _beginthreadex(0, 0, KaraPlayer::AudioThread, this, 0, 0);
}

void Suancai::Player::KaraPlayer::play(char8_t* pPath) {

    Message msg;
    msg.type = Message::Type::Play;
    msg.strData = pPath;
    this->toDecodeQ.push(msg);
}

void Suancai::Player::KaraPlayer::resume() {
}

void Suancai::Player::KaraPlayer::pause() {
}

void Suancai::Player::KaraPlayer::seek() {
}

u64 Suancai::Player::KaraPlayer::getPlaybackTime() {
    return u64();
}

u64 Suancai::Player::KaraPlayer::getPlaybackTimeDen() {
    return u64();
}

Suancai::Player::KaraPlayer::~KaraPlayer() {
}
