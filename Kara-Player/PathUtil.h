#pragma once

#include"__UtilCommon.h"

#include"BaseException.h"

#include<math.h>

#define PI_d 3.1415926535897932384626433832795
#define PI_f 3.1415926535897932384626433832795f

namespace Suancai {

	namespace Util {

		class PathUtil {
		public:
			static void GetParentFolder(std::u8string& path, std::u8string& outPath);
		};
	}
}