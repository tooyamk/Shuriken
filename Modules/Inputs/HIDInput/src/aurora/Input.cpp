#include "CreateModule.h"
#include "Gamepad.h"
#include "GamepadWindows.h"
#include "GamepadDS4.h"
#include "aurora/HID.h"
#include "aurora/hash/xxHash.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Input::Input(Ref* loader, IApplication* app, DeviceType filter) :
		_loader(loader),
		_app(app),
		_filter(filter),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()) {
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace aurora::extensions;
		using namespace aurora::enum_operators;

		if ((DeviceType::GAMEPAD & _filter) == DeviceType::UNKNOWN) return;

		std::vector<InternalDeviceInfo> newDevices;
		HID::enumDevices(&newDevices, [](const HIDDeviceInfo& info, void* custom) {
			auto newDevices = (std::vector<InternalDeviceInfo>*)custom;

			if (HID::getUsagePage(info) == 1) {
				if (HID::getUsage(info) == 5) {
					auto& dev = newDevices->emplace_back();

					auto path = HID::getPath(info);

					auto hash = hash::xxHash::calc<64, std::endian::native>(path.data(), path.size(), 0);
					dev.guid.set<false, true>(&hash, sizeof(hash), 0);
					dev.vendorID = HID::getVendorID(info);
					dev.productID = HID::getProductID(info);
					dev.type = DeviceType::GAMEPAD;
					dev.path = path;
				}
			}
		});

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
		using namespace aurora::extensions;

		std::shared_lock lock(_mutex);

		InternalDeviceInfo* di;
		HIDDevice* hid = nullptr;
		for (auto& info : _devices) {
			if (info.guid == guid) {
				di = &info;

				switch (info.type) {
				case DeviceType::GAMEPAD:
					hid = HID::open(info.path);
					break;
				default:
					break;
				}

				break;
			}
		}

		if (!hid) return nullptr;

		IInputDevice* device = nullptr;
		switch (di->vendorID) {
		case 0x54C:
		{
			if (di->productID == 0x5C4 || di->productID == 0x9CC) device = new GamepadDS4(*this, *di, *hid);

			break;
		}
		default:
			break;
		}

		if (!device) device = new Gamepad(*this, *di, *hid);

		return device;
	}
}