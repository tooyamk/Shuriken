#include "Input.h"
#include "CreateModule.h"
#include "GamepadDriver.h"
#include "srk/hash/xxHash.h"
#include <fcntl.h>
#include <linux/input.h>

namespace srk::modules::inputs::evdev {
	Input::Input(Ref* loader, const CreateInputModuleDesc& desc) :
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

				DeviceType type = DeviceType::UNKNOWN;
				std::string_view evt;
				String::split(line.substr(p + 9), String::CharFlag::WHITE_SPACE, [&evt, &type](const std::string_view& val){
					if (val.find("kbd"sv) != std::string_view::npos) {
						type |= DeviceType::KEYBOARD;
					} else if (val.find("mouse"sv) != std::string_view::npos) {
						type |= DeviceType::MOUSE;
					} else if (val.find("js"sv) != std::string_view::npos) {
						type |= DeviceType::GAMEPAD;
					} else if (val.find("event"sv) != std::string_view::npos) {
						evt = val;
					}
				});

				if (evt.empty() || (type & _filters) == DeviceType::UNKNOWN) continue;

				int32_t fd = -1;
				uint8_t state = 0;
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

					state = 1;
					auto& info = newDevices.emplace_back();
					info.vendorID = ids.vendor;
					info.productID = ids.product;
					info.type = type & _filters;
					memcpy(info.eventNumber, evt.data() + EVENT_STR_LEN, n);
					info.eventNumber[n] = 0;

					hash::xxHash<64> hasher;
					hasher.begin(0);
					hasher.update(evt.data() + EVENT_STR_LEN, n);
					hasher.update(&ids, sizeof(ids));

					char buf[256];

					if (ioctl(fd, EVIOCGNAME(sizeof(buf) - 1), buf) >= 0) info.name = buf;
					if (ioctl(fd, EVIOCGUNIQ(sizeof(buf) - 1), buf) >= 0) hasher.update(buf, strlen(buf));

					auto hash = hasher.digest();
					info.guid.set<false, true>(&hash, sizeof(hash), 0);

					state = 2;
				} while(false);

				if (fd >= 0) ::close(fd);
				if (state == 1) newDevices.pop_back();
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
			case DeviceType::GAMEPAD:
				if (auto driver = GamepadDriver::create(*this, fd); driver) return new GenericGamepad(info, *driver);
				break;
			default:
				break;
			}
		} while (false);

		ioctl(fd, EVIOCGRAB, 0);
		::close(fd);
		return nullptr;
	}
}