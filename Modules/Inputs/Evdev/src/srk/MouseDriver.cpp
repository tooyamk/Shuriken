#include "MouseDriver.h"
#include "Input.h"
#include "srk/Printer.h"
#include "srk/StringUtility.h"
#include <linux/input.h>

namespace srk::modules::inputs::evdev_input {
	MouseDriver::MouseDriver(Input& input, int32_t fd) :
		_fd(fd) {
	}

	MouseDriver::~MouseDriver() {
		close();
	}

	uint32_t MouseDriver::_instanceCount = 0;
	std::shared_mutex MouseDriver::_libMutex = {};
	DynamicLibraryLoader MouseDriver::_libLoader = {};
	MouseDriver::XOpenDisplayFn MouseDriver::_XOpenDisplayFn = nullptr;
	MouseDriver::XCloseDisplayFn MouseDriver::_XCloseDisplayFn = nullptr;
	MouseDriver::XQueryPointerFn MouseDriver::_XQueryPointerFn = nullptr;
	MouseDriver::XDisplay* MouseDriver::_xdisplay = nullptr;

	MouseDriver* MouseDriver::create(Input& input, int32_t fd) {
		{
			std::scoped_lock lock(_libMutex);

			if (++_instanceCount == 1) {
				if (_libLoader.load("libX11.so")) {
					do {
						_XOpenDisplayFn = (XOpenDisplayFn)_libLoader.getSymbolAddress("XOpenDisplay");
						_XCloseDisplayFn = (XCloseDisplayFn)_libLoader.getSymbolAddress("XCloseDisplay");
						_XQueryPointerFn = (XQueryPointerFn)_libLoader.getSymbolAddress("XQueryPointer");

						if (!_XOpenDisplayFn || !_XCloseDisplayFn || !_XQueryPointerFn) {
							_libLoader.release();
							break;
						}

						_xdisplay = _XOpenDisplayFn(nullptr);
						if (!_xdisplay) {
							_libLoader.release();
							break;
						}
					} while(false);
				}	
			}
		}

		return new MouseDriver(input, fd);
	}

	std::optional<bool> MouseDriver::readFromDevice(GenericMouseBuffer& buffer) const {
		using namespace std::string_view_literals;
		using namespace srk::enum_operators;
		
		input_event evts[8];
		auto changed = false;
		float32_t wheel = 0.f;

		if (auto pos = _getMousePos(); pos) {
			std::scoped_lock lock(_lock);

			if (_inputBuffer.pos != *pos) {
				_inputBuffer.pos = *pos;
				changed = true;
			}
		}

		do {
			auto len = read(_fd, evts, sizeof(evts));
			if (len < 0) break;

			if (len && (len % sizeof(input_event)) != 0) break;

			for (size_t i = 0, n = len / sizeof(input_event); i < n; ++i) {
				auto& evt = evts[i];

				switch (evt.type) {
				case EV_KEY:
				{
					if (evt.code >= BTN_MOUSE && evt.code <= BTN_TASK) {
						auto vk = MouseVirtualKeyCode::L_BUTTON + (evt.code - BTN_LEFT);
						if constexpr (Environment::IS_DEBUG) {
							if (!GenericMouseBuffer::isValidButton(vk)) {
								printaln(evt.code, L"     "sv, evt.value);
								break;
							}
						}

						auto val = evt.value != 0;
						if (_inputBuffer.setButton(vk, val, _lock)) changed = true;
					} else {
						//printaln(L"key  code:"sv, StringUtility::toString(evt.code, 16), L"  value:"sv, evt.value);
					}

					break;
				}
				case EV_REL:
				{
					switch (evt.code) {
						case REL_WHEEL:
						{
							changed = true;
							wheel += evt.value;

							break;
						}
						case REL_X:
						case REL_Y:
						case REL_HWHEEL:
						case REL_WHEEL_HI_RES:
						case REL_HWHEEL_HI_RES:
							break;
						default:
						{
							if constexpr (Environment::IS_DEBUG) {
								printaln(L"rel  code:"sv, StringUtility::toString(evt.code, 16), L"  value:"sv, evt.value);
							}

							break;
						}	
					}

					break;
				}
				case EV_ABS:
				case EV_SYN:
					break;
				default:
				{
					if constexpr (Environment::IS_DEBUG) {
						printaln(L"type:"sv, evt.type, L"  code:"sv, evt.code, L"  value:"sv, evt.value);
					}

					break;
				}
				}
			}
		} while (true);

		if (!changed) return std::make_optional(false);

		{
			std::scoped_lock lock(_lock);
			memcpy(&buffer, &_inputBuffer, sizeof(_inputBuffer));
		}

		buffer.wheel = wheel;

		return std::make_optional(true);
	}

	void MouseDriver::close() {
		if (_fd < 0) return;

		//ioctl(_desc.fd, EVIOCGRAB, 0);
		::close(_fd);
		_fd = -1;
		
		std::scoped_lock lock(_libMutex);

		if (--_instanceCount == 0) {
			if (_libLoader.isLoaded()) {
				_XCloseDisplayFn(_xdisplay);
				_xdisplay = nullptr;
				_libLoader.release();
			}
		}
	}

	std::optional<Vec2f32> MouseDriver::_getMousePos() {
		if (!_xdisplay) return std::nullopt;

		int ret = 0;
		int x = 0, y = 0;
		XID window = 0;
		XID root = 0;
		int dummyInt = 0;
		unsigned int dummyUint = 0;

		auto screenCount = _xdisplay->nscreens;
		for (decltype(screenCount) i = 0; i < screenCount; ++i) {
			ret = _XQueryPointerFn(_xdisplay, _xdisplay->screens[i].root, &root, &window, &x, &y, &dummyInt, &dummyInt, &dummyUint);
			if (ret != 0) break;
		}

		if (ret == 0) return std::nullopt;

		return std::make_optional(Vec2f32(x, y));
	}
}