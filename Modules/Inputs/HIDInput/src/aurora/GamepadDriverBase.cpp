#include "GamepadDriverBase.h"
#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	GamepadDriverBase::GamepadDriverBase(Input& input, extensions::HIDDevice& hid) :
		_input(input),
		_hid(&hid) {
	}

	GamepadDriverBase::~GamepadDriverBase() {
		using namespace aurora::extensions;

		HID::close(*_hid);
	}
}