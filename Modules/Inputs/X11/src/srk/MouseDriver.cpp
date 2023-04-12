#include "MouseDriver.h"
#include "Input.h"
#include "srk/Printer.h"

namespace srk::modules::inputs::x11 {
	MouseDriver::MouseDriver(Input& input, windows::IWindow& win) :
		_listener(input, win, DeviceType::MOUSE, _callback, this),
		_changed(false) {
		_listener.start();
	}

	MouseDriver::~MouseDriver() {
		close();
	}

	MouseDriver* MouseDriver::create(Input& input, windows::IWindow& win) {
		return new MouseDriver(input, win);
	}

	std::optional<bool> MouseDriver::readFromDevice(GenericMouseBuffer& buffer) const {
		{
			auto dis = (::Display*)_listener.getWindow()->getNative("XDisplay");
			if (!dis) return std::nullopt;

			int ret = 0;
			int x = 0, y = 0;
			::Window window = 0;
			::Window  root = 0;
			int dummyInt = 0;
			unsigned int dummyUint = 0;

			auto screenCount = ScreenCount(dis);
			for (decltype(screenCount) i = 0; i < screenCount; ++i) {
				ret = XQueryPointer(dis, RootWindowOfScreen(ScreenOfDisplay(dis, i)), &root, &window, &x, &y, &dummyInt, &dummyInt, &dummyUint);
				if (ret != 0) {
					if (_inputBuffer.setPos(Vec2f32(x, y), _lock)) _changed.store(true);

					break;
				}
			}
		}

		auto expected = true;
		if (_changed.compare_exchange_strong(expected, false, std::memory_order::release, std::memory_order::relaxed)) {
			{
				std::scoped_lock lock(_lock);

				memcpy(&buffer, &_inputBuffer, sizeof(_inputBuffer));
				_inputBuffer.wheel = 0.f;
			}

			return std::make_optional(true);
		}

		return std::make_optional(false);
	}

	void MouseDriver::close() {
		_listener.close();
	}

	void MouseDriver::_setWheel(float32_t val) {
		{
			std::scoped_lock lock(_lock);

			_inputBuffer.wheel += val;
		}

		_changed.store(true);
	}

	void MouseDriver::_setButton(MouseVirtualKeyCode vk, bool pressed) {
		if (_inputBuffer.setButton(vk, pressed, _lock)) _changed.store(true);
	}

	void MouseDriver::_callback(const XEvent& evt, void* target) {
		using namespace srk::enum_operators;

		if (evt.type == MotionNotify) return;

		auto driver = (MouseDriver*)target;

		auto pressed = evt.type == ButtonPress;
		switch (evt.xbutton.button) {
		case 1:
			driver->_setButton(MouseVirtualKeyCode::L_BUTTON, pressed);
			break;
		case 2:
			driver->_setButton(MouseVirtualKeyCode::M_BUTTON, pressed);
			break;
		case 3:
			driver->_setButton(MouseVirtualKeyCode::R_BUTTON, pressed);
			break;
		case 4:
			if (pressed) driver->_setWheel(1.f);
			break;
		case 5:
			if (pressed) driver->_setWheel(-1.f);
			break;
		case 8:
			driver->_setButton(MouseVirtualKeyCode::FN_BUTTON_1, pressed);
			break;
		case 9:
			driver->_setButton(MouseVirtualKeyCode::FN_BUTTON_1 + 1, pressed);
			break;
		default:
		{
			if constexpr (Environment::IS_DEBUG) {
				printaln(evt.xbutton.button);
			}
			
			break;
		}
		}
	}
}