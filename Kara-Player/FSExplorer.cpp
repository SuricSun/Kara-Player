#include "FSExplorer.h"
#include"UTFStringAffair.h"

bool Suancai::Util::FSExplorer::goToAbsolute(const char8_t* folder) {

    if (std::filesystem::is_directory(folder) == false) {
        return false;
    }
    this->currentFolder = folder;
    // * list
    this->listFilesIn(folder, this->files);
    return false;
}

bool Suancai::Util::FSExplorer::goToRelative(const char8_t* folderPath) {

    std::u8string actualPath = this->currentFolder.u8string().append(folderPath);
    if (std::filesystem::is_directory(actualPath) == false) {
        return false;
    }
    actualPath.append(u8"/");
    this->currentFolder = actualPath;
    // * list
    this->listFilesIn(actualPath.c_str(), this->files);
    return false;
}

bool Suancai::Util::FSExplorer::goback() {
    return false;
}

void Suancai::Util::FSExplorer::listFilesIn(const char8_t* folder, std::vector<std::pair<std::u8string, u8>>& files) {
    
    std::u8string u8folder = folder;
    std::u16string u16Folder;
    UTFStringAffair::UTF8To16(folder, u16Folder);

    WIN32_FIND_DATAW data;


    HANDLE h = FindFirstFileW((WCHAR*)u16Folder.append(u"*").c_str(), &data);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    u16string fileName;
    u8string out;
    files.clear();

    do {
        u8 type = -1;
        
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            type = 1;
        } else {
            type = 0;
        }

        fileName.clear();
        out.clear();
        fileName = (char16_t*)data.cFileName;
        UTFStringAffair::UTF16To8(fileName, out);
        files.push_back(std::make_pair(out, type));
    } while (FindNextFileW(h, &data));

    FindClose(h);
}

std::vector<std::pair<std::u8string, u8>>& Suancai::Util::FSExplorer::getFiles() {
    
    return this->files;
}
