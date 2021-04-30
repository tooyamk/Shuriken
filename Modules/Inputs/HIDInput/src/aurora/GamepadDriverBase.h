#pragma once

#include "Base.h"
#include "aurora/modules/inputs/GenericGamepad.h"

namespace aurora::modules::inputs::hid_input {
	class Input;

	class AE_MODULE_DLL GamepadDriverBase : public IGenericGamepadDriver {
	public:
		GamepadDriverBase(Input& input, extensions::HIDDevice& hid);
		virtual ~GamepadDriverBase();

	protected:
		IntrusivePtr<Input> _input;
		extensions::HIDDevice* _hid;
	};
}