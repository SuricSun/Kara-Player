#include "KaraPlayer.h"

using namespace Suancai;
using namespace Suancai::Media::Audio;

unsigned __stdcall Suancai::Player::KaraPlayer::AudioThread(void* pArg) {

    try {
        KaraPlayer* p = (KaraPlayer*)pArg;
        u32 free = 0, total = 0;
        bool start = false;
        i32 fftSize = 8192;
        float* pWindowed = new float[fftSize];
        FFT fft;
        fft.init(fftSize);
        i32 readSampleLogicalStart = 0;
        // * idle
        Message msg;
        while (true) {
            // * take message
            if (p->toAudioQ.pop(msg)) {
                switch (msg.type) {
                    case Message::Type::Play:
                    {
                        // * now open new file
                        AudioEnumerator emu;
                        emu.init();
                        p->audioRenderer.init(emu.getDefaultRenderDevice(), 10000000, false);
                        p->audioRenderer.enableRender();
                        p->vr.init();
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
            pWindowed[0] = p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + 0) % channelSamples];
            for (int i = 1; i < fftSize; i++) {
                pWindowed[i] =
                    pow((0.5f - (1.0f - 0.5f) * cos((2.0f * PI_F * i) / float((fftSize - 1)))), 2) *
                    (p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + i) % channelSamples] -
                        0.93f * p->audioDecoder.decodeCBuffer.buffer[0][(desiredLogicalRead + i - 1) % channelSamples]
                        - 0 * pWindowed[i - 1]);
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
            fft.doFFT(pWindowed, fftSize, 0);
            p->vr.render(fft.p_result1_final + (fftSize / 4), fftSize / 4);
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
            p->audioDecoder.decode(48000 * 10);
            // * then we sleep for half that length
            Sleep((10.0f / 2.0f) * 1000);
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
