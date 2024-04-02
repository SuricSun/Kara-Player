#include "PathUtil.h"

void Suancai::Util::PathUtil::GetParentFolder(std::u8string& path, std::u8string& outPath) {

	size_t pos = path.find_last_of(u8"/\\");
	if (pos == path.npos) {
		outPath = u8"";
	} else {
		outPath.clear();
		outPath.assign(path.cbegin(), path.cbegin() + pos + 1);
	}
}
