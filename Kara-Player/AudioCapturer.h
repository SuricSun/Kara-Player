#pragma once

#include"__AudioCommon.h"

#include"UTFStringAffair.h"

namespace Suancai {

	namespace Media {

		namespace Audio {

			using namespace Suancai::Util;

			class AudioCapturer {
			public:
				IMMDevice* p_device = nullptr;
				IAudioClient* p_client = nullptr;
				IAudioCaptureClient* pCaptureClient = nullptr;

				WAVEFORMATEXTENSIBLE* p_wave_fmt = nullptr;
				AudioSampleFormat audioFmt = {};

				u32 sampleBitDepth = 0;
				u32 channels = 0;
				u32 frameSizeInByte = 0;
				u32 samples_per_sec = 0;
				//AudioCaptureClient内部的捕获缓冲区实际大小
				u32 buffer_frame_cnt = 0;
				//name
				u16string device_name;
			public:
				AudioCapturer();
				/// <summary>
				/// 
				/// </summary>
				/// <param name="p_audDevice"></param>
				/// <param name="bufferDurationIn100NanoUnit">
				/// 10000000 means 1 sec
				/// which means alloc a buffer that contains 1 sec of audio data, buffer byte size is according to the current audio device output format
				/// </param>
				/// <param name="whetherLoopbackCapturing">if set to true, means capture stream from a render device, so p_audDevice should not be a capture device such as a microphone</param>
				void init(IMMDevice* p_audDevice, u32 bufferDurationIn100NanoUnit, bool whetherLoopbackCapturing);
				/// <summary>
				/// tell the system to start capture audio data, you can aquire the data from AudioCaptureClient
				/// </summary>
				void enableCapture();
				/// <summary>
				/// tell the system to stop capturing data
				/// </summary>
				void disable_capture();
				IAudioCaptureClient* getRenderClient();
				~AudioCapturer();
			};
		}
	}
}