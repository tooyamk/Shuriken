#include "KeyboardDriver.h"
#include "Input.h"
#include "srk/Printer.h"
#include <linux/input.h>

namespace srk::modules::inputs::evdev_input {
	KeyboardDriver::KeyboardDriver(Input& input, int32_t fd) :
		_fd(fd) {
	}

	KeyboardDriver::~KeyboardDriver() {
		::close(_fd);
	}

	KeyboardDriver* KeyboardDriver::create(Input& input, int32_t fd) {
		return new KeyboardDriver(input, fd);
	}

	bool KeyboardDriver::readStateFromDevice(GenericKeyboard::Buffer& buffer) const {
		using namespace std::string_view_literals;
		
		input_event evts[8];
		auto changed = false;
		do {
			auto len = read(_fd, evts, sizeof(evts));
			if (len < 0) break;

			if (len && (len % sizeof(input_event)) != 0) break;

			for (size_t i = 0, n = len / sizeof(input_event); i < n; ++i) {
				auto& evt = evts[i];

				switch (evt.type) {
				case EV_KEY:
				{
					if (evt.code >= KEY_ESC && evt.code <= KEY_MICMUTE) {
						auto vk = VK_MAPPER[evt.code];
						if (!GenericKeyboard::Buffer::isValid(vk)) {
							printaln(evt.code, L"     "sv, evt.value);
							break;
						}

						changed = true;
						auto val = evt.value != 0;

						std::scoped_lock lock(_lock);
						_inputBuffer.set(vk, val);
					} else {
						//printaln(L"key  code:"sv, String::toString(evt.code, 16), L"  value:"sv, evt.value);
					}

					break;
				}
				default:
					//printaln(L"type:"sv, evt.type, L"  code:"sv, evt.code, L"  value:"sv, evt.value);
					break;
				}
			}
		} while (true);

		if (!changed) return false;

		{
			std::scoped_lock lock(_lock);
			memcpy(buffer.data, _inputBuffer.data, sizeof(_inputBuffer.data));
		}

		return true;
	}
}