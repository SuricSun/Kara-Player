#pragma once

#include"__SuancaiCommon.h"

#include"AudioDecoder.h"
#include"AudioRenderer.h"
#include"AudioEnumerator.h"
#include"AudioSpecRenderer.h"

#include<queue>
#include<mutex>
#include<atomic>

namespace Suancai {

	namespace Player {

		using namespace Suancai;
		using namespace Suancai::Media::Audio;
		using namespace Suancai::Kara::Renderer;

		class Message {
		public:
			enum Type : u32 {
				None,
				Play,
				Resume,
				Pause,
				Seek,
				GimmeSamples
			};
			Type type = Type::None;
			u64 u64Data = 0;
			double doubleData = 0;
			void* pData = nullptr;
			std::u8string strData;
		};

		class MessageQueue {
		private:
			std::queue<Message> messages;
			std::mutex mtx;

		public:
			void push(const Message& msg) {
				std::lock_guard<std::mutex> lock(mtx);
				messages.push(msg);
			}
			bool pop(Message& msg) {
				std::lock_guard<std::mutex> lock(mtx);
				if (messages.empty()) {
					return false;
				}
				msg = messages.front();
				messages.pop();
				return true;
			}
		};

		class KaraPlayer {
		public:
		private:
			// * audio stuff
			MessageQueue toAudioQ;
			MessageQueue toDecodeQ;
			AudioDecoder audioDecoder;
			AudioRenderer audioRenderer;
			// * render stuff
			AudioSpecRenderer vr;
			// * shared stuff
		public:
			double time = 0;
			i32 sample = 0;
			KaraPlayer();
			void play(char8_t* pPath);
			void resume();
			void pause();
			void seek();
			u64 getPlaybackTime();
			u64 getPlaybackTimeDen();
			~KaraPlayer();
		public:
			static unsigned __stdcall AudioThread(void* pArg);
			static unsigned __stdcall DecodeThread(void* pArg);
		};
	}
}