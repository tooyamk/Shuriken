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

		static const std::filesystem::path dir("/dev/input/");
		if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
			for (auto& itr : std::filesystem::directory_iterator(dir)) {
				if (auto& path = itr.path(); path.has_filename() && std::string_view(path.filename().string()).substr(0, 5) == "event") {
					int32_t fd = -1;
					do {
						auto fd = ::open(path.string().c_str(), O_RDONLY|O_NONBLOCK);
						if (fd < 0) break;

						input_id ids;
						if (auto rc = ioctl(fd, EVIOCGID, &ids); rc < 0) break;

						printaln(L"bustype : "sv, ids.bustype, L"  vendor : "sv, ids.vendor, L"  product : "sv, ids.product, L"  version : "sv, ids.version);
					} while(false);

					if (fd >= 0) ::close(fd);
				}
			}
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