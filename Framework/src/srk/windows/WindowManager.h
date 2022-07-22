#pragma once

#include "srk/Global.h"

namespace srk {
	class IWindow;

	class SRK_FW_DLL WindowManager {
	public:
		void add(void* nativeWindow, IWindow* window);
		void remove(void* nativeWindow);

		void pollEvents();
		void sendEvent(void* nativeWindow, void* data);

	private:
		std::unordered_map<void*, IWindow*> _windows;
	};
}