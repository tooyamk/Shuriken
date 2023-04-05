#pragma once

#include "Base.h"
#include "srk/modules/inputs/GenericGamepad.h"

namespace srk::modules::inputs::hid_input {
	class Input;

	class SRK_MODULE_DLL GamepadDriverBase : public IGenericGamepadDriver {
	public:
		GamepadDriverBase(Input& input, extensions::HIDDevice& hid);
		virtual ~GamepadDriverBase();

		virtual void SRK_CALL close() override;

	protected:
		IntrusivePtr<Input> _input;
		extensions::HIDDevice* _hid;
	};
}