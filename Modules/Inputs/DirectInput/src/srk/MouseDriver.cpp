#include "MouseDriver.h"
#include "Input.h"
//#include "srk/Printer.h"

namespace srk::modules::inputs::direct_input {
	MouseDriver::MouseDriver(Input& input, srk_IDirectInputDevice* dev) :
		_input(input),
		_dev(dev) {
		_dev->SetDataFormat(&c_dfDIMouse2);
		_dev->SetCooperativeLevel(input.getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}

	MouseDriver::~MouseDriver() {
		close();
	}

	MouseDriver* MouseDriver::create(Input& input, srk_IDirectInputDevice* dev) {
		return new MouseDriver(input, dev);
	}

	std::optional<bool> MouseDriver::readFromDevice(GenericMouseBuffer& buffer) const {
		using namespace srk::enum_operators;

		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return std::nullopt;
			if (FAILED(_dev->Poll())) return std::nullopt;
		}

		DIMOUSESTATE2 state;
		if (SUCCEEDED(_dev->GetDeviceState(sizeof(DIMOUSESTATE2), &state))) {
			POINT p;
			GetCursorPos(&p);
			buffer.pos.set(p.x, p.y);

			constexpr float32_t wheelDelta = WHEEL_DELTA;
			buffer.wheel = state.lZ / wheelDelta;

			for (size_t i = 0; i < sizeof(DIMOUSESTATE2::rgbButtons); ++i) buffer.setButton(i + MouseVirtualKeyCode::BUTTON_START, state.rgbButtons[i] & 0x80, nullptr);

			return std::make_optional(true);
		}

		return std::nullopt;
	}

	void MouseDriver::close() {
		if (!_dev) return;

		_dev->Unacquire();
		_dev->Release();
		_dev = nullptr;
	}
}