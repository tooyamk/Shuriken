#pragma once

#include "srk/Global.h"
#include <functional>

namespace srk {
	class IWindow;

	class SRK_FW_DLL WindowManager {
	public:
		using EventFn = std::function<bool(IWindow*, void*)>;

		void add(void* nativeWindow, IWindow* window);
		void remove(void* nativeWindow);

		bool processEvent() const;
		bool processEvent(const EventFn& fn) const;
		bool sendEvent(void* nativeWindow, void* data, const EventFn& fn) const;

	private:
		std::unordered_map<void*, IWindow*> _windows;
	};
}