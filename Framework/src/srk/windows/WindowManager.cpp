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

	bool WindowManager::processEvent() const {
		return processEvent([](IWindow* win, void* data) {
			if (!win) return true;
			
			win->processEvent(data);
			return false;
		});
	}

	bool WindowManager::sendEvent(void* nativeWindow, void* data, const EventFn& fn) const {
		auto itr = _windows.find(nativeWindow);
		return fn(itr == _windows.end() ? nullptr : itr->second, data);
	}
}