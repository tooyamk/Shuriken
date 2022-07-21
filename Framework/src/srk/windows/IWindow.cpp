#include "IWindow.h"

namespace srk {
	std::unordered_map<void*, IWindow*> IWindow::_windows = std::unordered_map<void*, IWindow*>();

	void IWindow::_register(void* nativeWindow, IWindow* window) {
		if (!nativeWindow || !window) return;

		_windows.insert_or_assign(nativeWindow, window);
	}

	void IWindow::_unregister(void* nativeWindow) {
		if (!nativeWindow) return;

		_windows.erase(nativeWindow);
	}
}