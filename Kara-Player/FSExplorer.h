#pragma once

#include"__UtilCommon.h"

#include<vector>;
#include<filesystem>

namespace Suancai {

	namespace Util {
		
		class FSExplorer {
		public:
			std::filesystem::path currentFolder;
			std::vector<std::pair<std::u8string, u8>> files;
		public:
			bool goToAbsolute(const char8_t* folderPath);
			bool goToRelative(const char8_t* folderPath);
			bool goback();
			void listFilesIn(const char8_t* folder, std::vector<std::pair<std::u8string, u8>>& files);
			std::vector<std::pair<std::u8string, u8>>& getFiles();
		};
	}
}