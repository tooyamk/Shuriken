#pragma once

#include "InputListener.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericMouse.h"

namespace srk::modules::inputs::raw_input {
	class SRK_MODULE_DLL MouseDriver : public IGenericMouseDriver {
	public:
		virtual ~MouseDriver();

		static MouseDriver* SRK_CALL create(Input& input, windows::IWindow& win, HANDLE handle);

		virtual std::optional<bool> SRK_CALL readFromDevice(GenericMouseBuffer& buffer) const override;
		virtual void SRK_CALL close() override;

	private:
		MouseDriver(Input& input, windows::IWindow& win, HANDLE handle);

		InputListener _listener;

		mutable AtomicLock<true, false> _lock;
		mutable GenericMouseBuffer _inputBuffer;
		mutable std::atomic_bool _changed;

		void SRK_CALL _setButton(MouseVirtualKeyCode vk, bool pressed);
		static void SRK_CALL _callback(const RAWINPUT& rawInput, void* target);
	};
}