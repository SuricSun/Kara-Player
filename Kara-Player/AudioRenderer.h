#pragma once

#include"__AudioCommon.h"

#include"UTFStringAffair.h"

namespace Suancai {

	namespace Media {

		namespace Audio {

			using namespace Suancai::Util;

			class AudioRenderer {
			protected:
				IMMDevice* p_device = nullptr;
				IAudioClient* p_client = nullptr;
				IAudioRenderClient* pRenderClient = nullptr;
				IAudioClock* pClock = nullptr;

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
				//
				std::pair<u64, u64> devicePosition;
			public:
				AudioRenderer();
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
				void enableRender();
				void play(std::vector<std::vector<float>>& data, i32 samples);
				std::pair<u64, u64>& getDevicePosition();
				void getFreeSpaceCnt(u32& freeCnt, u32& totalCnt);
				void* getFreeSpacePointer(i32 frameRequested);
				void releaseFreeSpacePointer(i32 frameWrote);
				/// <summary>
				/// tell the system to stop capturing data
				/// </summary>
				void disableRender();
				IAudioRenderClient* getRenderClient();
				~AudioRenderer();
			};
		}
	}
}