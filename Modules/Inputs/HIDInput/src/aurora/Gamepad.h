#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public GamepadBase<42, 42> {
	public:
		Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual Key::CountType AE_CALL getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const override;

	protected:
		virtual void AE_CALL _parse(bool dispatchEvent, ReadBuffer& readBuffer, size_t readBufferSize) override;
	};
}