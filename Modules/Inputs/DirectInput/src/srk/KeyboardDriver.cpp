#include "KeyboardDriver.h"
#include "Input.h"

namespace srk::modules::inputs::direct_input {
	KeyboardDriver::KeyboardDriver(Input& input, srk_IDirectInputDevice* dev) :
		_input(input),
		_dev(dev) {
		_dev->SetDataFormat(&c_dfDIKeyboard);
		_dev->SetCooperativeLevel(input.getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}

	KeyboardDriver::~KeyboardDriver() {
		_dev->Unacquire();
		_dev->Release();
	}

	KeyboardDriver* KeyboardDriver::create(Input& input, srk_IDirectInputDevice* dev) {
		return new KeyboardDriver(input, dev);
	}

	bool KeyboardDriver::readStateFromDevice(GenericKeyboard::Buffer& buffer) const {
		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return false;
			if (FAILED(_dev->Poll())) return false;
		}

		uint8_t buf[256];
		if (SUCCEEDED(_dev->GetDeviceState(sizeof(buf), buf))) {
			for (size_t i = 0; i < sizeof(buf); ++i) buffer.set(SK_VK[i], buf[i] & 0x80);

			return true;
		}

		return false;
	}
}