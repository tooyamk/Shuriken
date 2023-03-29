#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs::raw_input {
	class Input;

	class SRK_MODULE_DLL InputListener {
	public:
		using Callback = void(SRK_CALL*)(const RAWINPUT&, void*);

		InputListener(Input& input, windows::IWindow& win, HANDLE handle, DeviceType type, Callback callback, void* callbackTarget);
		~InputListener();

		inline HWND SRK_CALL getHWND() const {
			return (HWND)_win->getNative(windows::WindowNative::WINDOW);
		}

		void SRK_CALL start();

	private:
		IntrusivePtr<Input> _input;
		IntrusivePtr<windows::IWindow> _win;
		HANDLE _handle;
		DeviceType _type;
		bool _listening;
		Callback _callback;
		void* _callbackTarget;

		events::EventListener<windows::WindowEvent, events::EvtMethod<windows::WindowEvent, InputListener>> _rawIputHandler;

		void SRK_CALL _rawInputCallback(events::Event<windows::WindowEvent>& e);
	};
}