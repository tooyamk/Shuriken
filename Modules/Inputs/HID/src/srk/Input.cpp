#include "CreateModule.h"
#include "GamepadDriver.h"
#include "GamepadDriverDS4.h"
#include "srk/hash/xxHash.h"

namespace srk::modules::inputs::hid_input {
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
		using namespace srk::extensions;
		using namespace srk::enum_operators;

		if ((DeviceType::GAMEPAD & _filters) == DeviceType::UNKNOWN) return;

		std::vector<InternalDeviceInfo> newDevices;
		HID::enumDevices(&newDevices, [](const HIDDeviceInfo& info, void* userData) {
			auto newDevices = (std::vector<InternalDeviceInfo>*)userData;

			auto usagePage = HID::getUsagePage(info);
			auto usage = HID::getUsage(info);
			if (usagePage == HIDReportUsagePageType::GENERIC_DESKTOP && (usage == HIDReportGenericDesktopPageType::JOYSTICK || usage == HIDReportGenericDesktopPageType::GAMEPAD)) {
				auto& dev = newDevices->emplace_back();

				auto path = HID::getPath(info);

				auto hash = hash::xxHash<64>::calc(path.data(), path.size(), 0);
				dev.index = HID::getIndex(info);
				dev.guid.set<false, false>(&hash, sizeof(hash), 0);
				dev.guid.set<false, true>(&dev.index, sizeof(dev.index), sizeof(hash));
				dev.vendorID = HID::getVendorID(info);
				dev.productID = HID::getProductID(info);
				dev.type = DeviceType::GAMEPAD;
				dev.path = path;
				dev.name = HID::getProductString(info);
			}

			return true;
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
		using namespace srk::extensions;

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

		auto hid = HID::open(info.path);
		if (!hid) return nullptr;

		IGenericGamepadDriver* driver = nullptr;
		switch (info.vendorID << 16 | info.productID) {
		case 0x54C << 16 | 0x5C4:
		case 0x54C << 16 | 0x9CC:
			driver = new GamepadDriverDS4(*this, *hid);
			break;
		default:
			break;
		}

		if (!driver) driver = GamepadDriver::create(*this, *hid, info.index);
		if (driver) return new GenericGamepad(info, *driver);

		HID::close(*hid);
		return nullptr;
	}
}