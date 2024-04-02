#pragma once

#include"__ExceptionCommon.h"

#include<sstream>

namespace Suancai {

	namespace Exception {

		class BaseException {
		protected:
			std::u16string className = u"BaseException";
			std::u16string info;
			std::u16string fileName;
			std::u16string funcName;
			i32 lineNumber = -1;
			i32 code = -1;
		public:
			BaseException() {};
			BaseException* setInfo(const char16_t* pInfo) { this->info.clear(); this->info.assign(pInfo); return this; };
			BaseException* setFileName(const char16_t* pFileName) { this->fileName.clear(); this->fileName.assign(pFileName); return this; };
			BaseException* setFuncName(const char16_t* pFuncName) { this->funcName.clear(); this->funcName.assign(pFuncName); return this; };
			BaseException* setLineNumber(i32 lineNumber) { this->lineNumber = lineNumber; return this; };
			BaseException* setCode(i32 code) { this->code = code; return this; };
			void showMsg() { 
				std::u16string str;
				
				str.append(u"[INFO]\n");
				str.append(this->info);
				str.append(u"\n");

				str.append(u"[FILE]\n");
				str.append(this->fileName);
				str.append(u"\n");

				str.append(u"[FUNC]\n");
				str.append(this->funcName);
				str.append(u"\n");

				str.append(u"[LINE]\n");
				str.append((char16_t*)std::to_wstring(this->lineNumber).c_str());
				str.append(u"\n");

				str.append(u"[CODE]\n");
				str.append((char16_t*)std::to_wstring(this->code).c_str());
				str.append(u"\n");

				MessageBoxW(NULL, (WCHAR*)str.c_str(), (WCHAR*)this->className.c_str(), MB_OK | MB_ICONWARNING | MB_TOPMOST);
			};
			~BaseException() {};
		};
	}
}
