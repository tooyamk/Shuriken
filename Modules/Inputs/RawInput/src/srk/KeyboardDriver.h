#pragma once

#include "DeviceBase.h"
#include "InputListener.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericKeyboard.h"

namespace srk::modules::inputs::raw_input {
	class SRK_MODULE_DLL KeyboardDriver : public IGenericKeyboardDriver {
	public:
		virtual ~KeyboardDriver();

		static KeyboardDriver* SRK_CALL create(Input& input, windows::IWindow& win, HANDLE handle);

		virtual bool SRK_CALL readStateFromDevice(GenericKeyboard::Buffer& buffer) const override;

	private:
		KeyboardDriver(Input& input, windows::IWindow& win, HANDLE handle);

		InputListener _listener;

		mutable AtomicLock<true, false> _lock;
		GenericKeyboard::Buffer _inputBuffer;
		std::atomic_bool _changed;

		static void SRK_CALL _callback(const RAWINPUT& rawInput, void* target);
		static KeyboardVirtualKeyCode SRK_CALL _getVirtualKey(const RAWKEYBOARD& raw);
	};
}