#include "Input.h"
#include "CreateModule.h"
#include "GamepadDriver.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "srk/StringUtility.h"
#include "srk/hash/xxHash.h"
#include <fcntl.h>
#include <linux/input.h>

namespace srk::modules::inputs::evdev_input {
	Input::Input(Ref* loader, const CreateInputModuleDescriptor& desc) :
		_loader(loader),
		_win(desc.window),
		_filters(desc.filters),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()) {
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace std::literals;
		using namespace srk::enum_operators;

		std::vector<InternalDeviceInfo> newDevices;

		if (auto devs = fopen("/proc/bus/input/devices", "r"); devs) {
			char devsBuf[512];
			char path[] = SRK_MODULE_INPUT_EVDEV_PATH_BUFFER;
			
			while (fgets(devsBuf, sizeof(devsBuf), devs)) {
				std::string_view line(devsBuf);

				auto p = line.find("Handlers="); 
				if (p == decltype(line)::npos) continue;

				std::string_view evt;
				StringUtility::split(line.substr(p + 9), StringUtility::CharFlag::WHITE_SPACE, [&evt](const std::string_view& val){
					if (val.find("event"sv) != std::string_view::npos) evt = val;
				});

				if (evt.empty()) continue;

				int32_t fd = -1;
				do {
					constexpr auto dstBegin = sizeof(path) - EVENT_NUMBER_BUFFER_LEN - 1;
					auto n = evt.size() - EVENT_STR_LEN;
					if (n > EVENT_NUMBER_BUFFER_LEN) break;
					memcpy(path + dstBegin, evt.data() + EVENT_STR_LEN, n);
					path[dstBegin + n] = 0;

					fd = ::open(path, O_RDONLY | O_NONBLOCK);
					if (fd < 0) break;

					input_id ids;
					if (auto rc = ioctl(fd, EVIOCGID, &ids); rc < 0) break;

					DeviceType types = DeviceType::UNKNOWN;
					{
						uint8_t bits[(std::max(ABS_CNT, KEY_CNT) + 7) >> 3];

						auto len = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(bits)), bits); 
						if (len < 0) break;

						struct {
							bool hasAbsXY = false;
						} caps;

						traverseBits(bits, len, 0, [](size_t bit, DeviceType& types, decltype(caps)& caps) {
							if (bit >= ABS_X && bit <= ABS_Y) {
								caps.hasAbsXY = true;
								return ABS_Y + 1;
							}

							if (bit >= ABS_Z && bit <= ABS_RZ) {
								types |= DeviceType::GAMEPAD;
								return ABS_RZ + 1;
							}

							if (bit >= ABS_THROTTLE && bit <= ABS_BRAKE) return ABS_BRAKE + 1;

							if (bit >= ABS_HAT0X && bit <= ABS_HAT3Y) {
								types |= DeviceType::GAMEPAD;
								return ABS_HAT3Y + 1;
							}

							if (bit >= ABS_PRESSURE && bit <= ABS_MAX) return ABS_MAX + 1;

							return 0;
						}, types, caps);

						len = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits); 
						if (len < 0) break;

						traverseBits(bits, len, 0, [](size_t bit, DeviceType& types) {
							if (bit >= KEY_ESC && bit <= KEY_MICMUTE) {
								types |= DeviceType::KEYBOARD;
								return KEY_MICMUTE + 1;
							}

							if (bit >= BTN_MISC && bit <= BTN_9) return BTN_9 + 1;

							if (bit >= BTN_MOUSE && bit <= BTN_TASK) {
								types |= DeviceType::MOUSE;
								return BTN_TASK + 1;
							}

							if (bit >= BTN_JOYSTICK && bit <= BTN_DEAD) return BTN_DEAD + 1;

							if (bit >= BTN_GAMEPAD && bit <= BTN_THUMBR) {
								types |= DeviceType::GAMEPAD;
								return BTN_THUMBR + 1;
							}

							if (bit >= BTN_DIGI && bit <= KEY_MAX) return KEY_MAX + 1;

							return 0;
						}, types);
					}

					types &= _filters;

					auto type = (DeviceType)1;
					while (types != DeviceType::UNKNOWN) {
						if ((types & type) == DeviceType::UNKNOWN) {
							type <<= 1;

							continue;
						}

						types &= ~type;

						auto& info = newDevices.emplace_back();
						info.vendorID = ids.vendor;
						info.productID = ids.product;
						info.type = type;
						memcpy(info.eventNumber, evt.data() + EVENT_STR_LEN, n);
						info.eventNumber[n] = 0;

						hash::xxHash<64> hasher;
						hasher.begin(0);
						hasher.update(evt.data() + EVENT_STR_LEN, n);
						hasher.update(&ids, sizeof(ids));
						hasher.update(&type, sizeof(type));

						char buf[256];

						if (ioctl(fd, EVIOCGNAME(sizeof(buf) - 1), buf) >= 0) info.name = buf;
						if (ioctl(fd, EVIOCGUNIQ(sizeof(buf) - 1), buf) >= 0) hasher.update(buf, strlen(buf));

						auto hash = hasher.digest();
						info.guid.set<false, true>(&hash, sizeof(hash), 0);
					}
				} while(false);

				if (fd >= 0) ::close(fd);
			}

			fclose(devs);
		}

		std::vector<DeviceInfo> add;
		std::vector<DeviceInfo> remove;
		{
			std::scoped_lock lock(_mutex);

			for (auto& info : newDevices) {
				if (!_hasDevice(info, _devices)) add.emplace_back(info);
			}

			for (auto& info : _devices) {
				if (!_hasDevice(info, newDevices)) remove.emplace_back(info);
			}

			_devices = std::move(newDevices);
		}

		for (auto& info : remove) _eventDispatcher->dispatchEvent(this, ModuleEvent::DISCONNECTED, &info);
		for (auto& info : add) _eventDispatcher->dispatchEvent(this, ModuleEvent::CONNECTED, &info);
	}

	IntrusivePtr<IInputDevice> Input::createDevice(const DeviceGUID& guid) {
		InternalDeviceInfo info;
		auto found = false;

		{
			std::shared_lock lock(_mutex);

			for (auto& i : _devices) {
				if (i.guid == guid) {
					info = i;
					found = true;

					break;
				}
			}
		}

		if (!found) return nullptr;

		char path[] = SRK_MODULE_INPUT_EVDEV_PATH_BUFFER;
		for (size_t i = 0; ; ++i) {
			path[sizeof(path) - EVENT_NUMBER_BUFFER_LEN - 1 + i] = info.eventNumber[i];
			if (info.eventNumber[i] == 0) break;
		}

		auto fd = ::open(path, O_RDONLY | O_NONBLOCK);
		if (fd < 0) return nullptr;
		
		do {
			//if (ioctl(fd, EVIOCGRAB, 1) < 0) break;

			switch (info.type) {
			case DeviceType::KEYBOARD:
				if (auto driver = KeyboardDriver::create(*this, fd); driver) return new GenericKeyboard(info, *driver);
				break;
			case DeviceType::MOUSE:
				if (auto driver = MouseDriver::create(*this, fd); driver) return new GenericMouse(info, *driver);
				break;
			case DeviceType::GAMEPAD:
				if (auto driver = GamepadDriver::create(*this, fd); driver) return new GenericGamepad(info, *driver);
				break;
			default:
				break;
			}
		} while (false);

		//ioctl(fd, EVIOCGRAB, 0);
		::close(fd);
		return nullptr;
	}
}