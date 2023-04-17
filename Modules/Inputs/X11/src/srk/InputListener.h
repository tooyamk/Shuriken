#pragma once

#include "Base.h"
#include "srk/Lock.h"
#include "srk/events/EventDispatcher.h"
#include <X11/Xlib.h>

namespace srk::modules::inputs::x11 {
	class Input;

	class SRK_MODULE_DLL InputListener {
	public:
		using Callback = void(SRK_CALL*)(const XEvent&, void*);

		InputListener(Input& input, windows::IWindow& win, DeviceType type, Callback callback, void* callbackTarget);
		~InputListener();

		inline windows::IWindow* getWindow() const {
			return _win;
		}

		void SRK_CALL start();
		void SRK_CALL close();

	private:
		IntrusivePtr<Input> _input;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _type;
		mutable AtomicLock _lock;
		bool _listening;
		Callback _callback;
		void* _callbackTarget;

		events::EventListener<windows::WindowEvent, events::EvtMethod<windows::WindowEvent, InputListener>> _iputHandler;

		void SRK_CALL _inputCallback(events::Event<windows::WindowEvent>& e);
	};
}