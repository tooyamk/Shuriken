#include "GamepadDriverBase.h"
#include "Input.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriverBase::GamepadDriverBase(Input& input, extensions::HIDDevice& hid) :
		_input(input),
		_hid(&hid) {
	}

	GamepadDriverBase::~GamepadDriverBase() {
		close();
	}

	void GamepadDriverBase::close() {
		using namespace srk::extensions;

		if (!_hid) return;
		HID::close(*_hid);
		_hid = nullptr;
	}
}