#pragma once

#include"__RendererCommon.h"

#include"SuancaiRenderer.h"
#include"FFT.h"

#include<vector>

#define CoordMap(x, s) ((s + 1.0f) / s) * (-1.0f / (s * x + 1.0f) + 1.0f)
// * x = [0, +), rate = (0, +) where higher rate mean quicker close to dest, reachWhere = (-, +)
#define NeverReach(x, rate, reachWhere) (((-1.0f / (rate * x + 1.0f)) + 1.0f) * reachWhere)

namespace Suancai {

	namespace Kara {

		namespace Renderer {

			class AudioSpecRenderer {
			protected:
				HWND hwnd = NULL;
				SuancaiRenderer rdr;
				GraphicsObject go;

				std::vector<float>* pCurProcessedData;
				std::vector<float>* pNxtProcessedData;
				std::vector<float> processedData0;
				std::vector<float> processedData1;
				float curTime = 0;
			public:
				void init();
				void clear();
				void setAlphaBlend(bool enable);
				void render(float* pAudioL, float* pAudioR, Suancai::Util::FFT::Complex* pSpec, i32 cnt, bool isLeft);
				void blurAndPresent();
			};
		}
	}
}