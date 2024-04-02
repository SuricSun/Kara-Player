#pragma once

#include"__RendererCommon.h"

#include"SuancaiRenderer.h"
#include"FFT.h"

#include<vector>

#define CoordMap(x, s) ((s + 1.0f) / s) * (-1.0f / (s * x + 1.0f) + 1.0f)

namespace Suancai {

	namespace Kara {

		namespace Renderer {

			class AudioSpecRenderer {
			protected:
				HWND hwnd = NULL;
				SuancaiRenderer rdr;
				GraphicsObject go;
				std::vector<Suancai::Util::FFT::Complex> specData;
			public:
				void init();
				void render(Suancai::Util::FFT::Complex* pSpec, i32 cnt);
			};
		}
	}
}