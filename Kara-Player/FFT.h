#pragma once

#include"__UtilCommon.h"

#include"BaseException.h"

#include<math.h>

#define PI_d 3.1415926535897932384626433832795
#define PI_f 3.1415926535897932384626433832795f

namespace Suancai {

	namespace Util {

		class FFT {
		public:
			struct Complex {
				float real = 0;
				float imagine = 0;
			};
		public:
			u32 fft_size = 0;
			Complex* p_result0 = nullptr;
			Complex* p_result1_final = nullptr;
		protected:
		public:
			FFT();
			void init(u32 fft_size);
			void doFFT(float* p_samples, u32 sample_size, u32 sample_offset);
			void do_fft_inner(float* samples, u32 sample_cnt, u32 sample_offset, u32 sample_start, u32 sample_step, u32 step_cnt, Complex* p_buffer_read, Complex* p_buffer_write);
			void doIFFT(Complex* pFFTs, u32 fftSize);
			~FFT();
		};
	}
}