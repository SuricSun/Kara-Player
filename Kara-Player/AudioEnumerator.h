#pragma once

#include"__AudioCommon.h"

namespace Suancai {

	namespace Media {

		namespace Audio {

			using namespace Microsoft::WRL;
			using namespace std;

			class AudioEnumerator {
			public:
				IMMDeviceEnumerator* p_enumerator = nullptr;
				vector<ComPtr<IMMDevice>> p_renderDeviceVec;
				vector<ComPtr<IMMDevice>> p_captureDeviceVec;
			public:
				AudioEnumerator();
				void init();
				void enumAudioDevice();
				IMMDevice* getDefaultRenderDevice();
				IMMDevice* getDefaultCaptureDevice();
				~AudioEnumerator();
			};
		}
	}
}