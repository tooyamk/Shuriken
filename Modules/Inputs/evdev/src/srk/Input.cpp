#include "Input.h"
#include "CreateModule.h"
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
				static char path[] = {"/dev/input/eventxxx" };
				do {
					constexpr auto dstBegin = sizeof(path) - 4;
					auto n = evt.size() - 5;
					memcpy(path + dstBegin, evt.data() + 5, n);
					path[dstBegin + n] = 0;

					fd = ::open(path, O_RDONLY|O_NONBLOCK);
					if (fd < 0) break;

					input_id ids;
					if (auto rc = ioctl(fd, EVIOCGID, &ids); rc < 0) break;

					state = 1;
					auto& info = newDevices.emplace_back();
					info.vendorID = ids.vendor;
					info.productID = ids.product;
					info.type = type & _filters;

					hash::xxHash<64> hasher;
					hasher.begin(0);
					hasher.update(evt.data() + 5, n);
					hasher.update(&ids, sizeof(ids));

					char buf[256];

					if (auto rc = ioctl(fd, EVIOCGNAME(sizeof(buf) - 1), buf); rc >= 0) {
						info.name = buf;
						hasher.update(info.name.data(), info.name.size());
					}
					
					if (auto rc = ioctl(fd, EVIOCGUNIQ(sizeof(buf) - 1), buf); rc < 0) hasher.update(buf, strlen(buf));

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
		return nullptr;
	}
}