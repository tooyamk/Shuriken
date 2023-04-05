#include "MouseDriver.h"
#include "Input.h"
//#include "srk/Printer.h"

namespace srk::modules::inputs::raw_input {
	MouseDriver::MouseDriver(Input& input, windows::IWindow& win, HANDLE handle) :
		_listener(input, win, handle, DeviceType::MOUSE, &MouseDriver::_callback, this),
		_changed(false) {
		_listener.start();
	}

	MouseDriver::~MouseDriver() {
		close();
	}

	MouseDriver* MouseDriver::create(Input& input, windows::IWindow& win, HANDLE handle) {
		return new MouseDriver(input, win, handle);
	}

	std::optional<bool> MouseDriver::readFromDevice(GenericMouseBuffer& buffer) const {
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

	void MouseDriver::_setButton(MouseVirtualKeyCode vk, bool pressed) {
		if (!GenericMouseBuffer::isValidButton(vk)) return;

		{
			std::scoped_lock lock(_lock);
			_inputBuffer.setButton(vk, pressed);
		}

		_changed.store(true);
	}

	void MouseDriver::_callback(const RAWINPUT& rawInput, void* target) {
		using namespace srk::enum_operators;

		auto driver = (MouseDriver*)target;
		auto& m = rawInput.data.mouse;

		{
			POINT p;
			GetCursorPos(&p);
			Vec2f32 pos(p.x, p.y);

			driver->_lock.lock();
			if (driver->_inputBuffer.pos != pos) {
				driver->_inputBuffer.pos = pos;
				driver->_lock.unlock();
				driver->_changed.store(true);
			} else {
				driver->_lock.unlock();
			}
		}

		constexpr size_t n = sizeof(m.usButtonFlags) << 3;
		for (size_t i = 0; i < n; ++i) {
			decltype(m.usButtonFlags) flag = 1 << i;
			if ((m.usButtonFlags & flag) != flag) continue;

			switch (flag) {
			case RI_MOUSE_LEFT_BUTTON_DOWN:
				driver->_setButton(MouseVirtualKeyCode::L_BUTTON, true);
				break;
			case RI_MOUSE_LEFT_BUTTON_UP:
				driver->_setButton(MouseVirtualKeyCode::L_BUTTON, false);
				break;
			case RI_MOUSE_RIGHT_BUTTON_DOWN:
				driver->_setButton(MouseVirtualKeyCode::R_BUTTON, true);
				break;
			case RI_MOUSE_RIGHT_BUTTON_UP:
				driver->_setButton(MouseVirtualKeyCode::R_BUTTON, false);
				break;
			case RI_MOUSE_MIDDLE_BUTTON_DOWN:
				driver->_setButton(MouseVirtualKeyCode::M_BUTTON, true);
				break;
			case RI_MOUSE_MIDDLE_BUTTON_UP:
				driver->_setButton(MouseVirtualKeyCode::M_BUTTON, false);
				break;
			case RI_MOUSE_BUTTON_4_DOWN:
				driver->_setButton(MouseVirtualKeyCode::FN_BUTTON_1, true);
				break;
			case RI_MOUSE_BUTTON_4_UP:
				driver->_setButton(MouseVirtualKeyCode::FN_BUTTON_1, false);
				break;
			case RI_MOUSE_BUTTON_5_DOWN:
				driver->_setButton(MouseVirtualKeyCode::FN_BUTTON_1 + 1, true);
				break;
			case RI_MOUSE_BUTTON_5_UP:
				driver->_setButton(MouseVirtualKeyCode::FN_BUTTON_1 + 1, false);
				break;
			case RI_MOUSE_WHEEL:
			{
				auto wheel = (SHORT)m.usButtonData / (float32_t)WHEEL_DELTA;

				{
					std::scoped_lock lock(driver->_lock);
					driver->_inputBuffer.wheel += wheel;
				}
				driver->_changed.store(true);

				break;
			}
			default:
				break;
			}
		}
	}
}