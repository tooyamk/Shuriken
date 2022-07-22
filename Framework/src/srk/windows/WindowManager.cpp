#include "WindowManager.h"
#include "srk/windows/IWindow.h"

namespace srk {
	void WindowManager::add(void* nativeWindow, IWindow* window) {
		if (!nativeWindow || !window) return;

		_windows.insert_or_assign(nativeWindow, window);
	}

	void WindowManager::remove(void* nativeWindow) {
		if (!nativeWindow) return;

		_windows.erase(nativeWindow);
	}

	void WindowManager::sendEvent(void* nativeWindow, void* data) {
		if (auto itr = _windows.find(nativeWindow); itr != _windows.end()) itr->second->processEvent(data);
	}
}