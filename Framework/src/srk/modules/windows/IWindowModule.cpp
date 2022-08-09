#pragma once

#include "IWindowModule.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::windows {
	bool DefaultWindowModule::processEvent() const {
		return processEvent([](IWindow* win, void* data) {
			if (!win) return true;

			win->processEvent(data);
			return false;
			});
	}

	bool DefaultWindowModule::sendEvent(void* nativeWindow, void* data, const IWindowModule::EventFn& fn) const {
		auto itr = _windows.find(nativeWindow);
		return fn(itr == _windows.end() ? nullptr : itr->second, data);
	}

	void DefaultWindowModule::_add(void* nativeWindow, IWindow* window) {
		if (!nativeWindow || !window) return;

		_windows.insert_or_assign(nativeWindow, window);
	}

	void DefaultWindowModule::_remove(void* nativeWindow) {
		if (!nativeWindow) return;

		_windows.erase(nativeWindow);
	}
}