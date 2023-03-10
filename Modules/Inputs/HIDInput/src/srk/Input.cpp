#include "CreateModule.h"
#include "GamepadDriver.h"
#include "GamepadDriverWindows.h"
#include "GamepadDriverDS4.h"
#include "srk/HID.h"
#include "srk/hash/xxHash.h"

namespace srk::modules::inputs::hid_input {
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
		using namespace srk::extensions;
		using namespace srk::enum_operators;

		if ((DeviceType::GAMEPAD & _filters) == DeviceType::UNKNOWN) return;

		std::vector<InternalDeviceInfo> newDevices;
		HID::enumDevices(&newDevices, [](const HIDDeviceInfo& info, void* custom) {
			auto newDevices = (std::vector<InternalDeviceInfo>*)custom;

			if (HID::getUsagePage(info) == HIDReportUsagePageType::GENERIC_DESKTOP) {
				if (auto usage = HID::getUsage(info); usage == HIDReportGenericDesktopPageType::JOYSTICK || usage == HIDReportGenericDesktopPageType::GAMEPAD) {
					auto& dev = newDevices->emplace_back();

					auto path = HID::getPath(info);

					auto hash = hash::xxHash<64>::calc<std::endian::native>(path.data(), path.size(), 0);
					dev.guid.set<false, true>(&hash, sizeof(hash), 0);
					dev.vendorID = HID::getVendorID(info);
					dev.productID = HID::getProductID(info);
					dev.type = DeviceType::GAMEPAD;
					dev.path = path;
					dev.name = HID::getProductString(info);

					switch (dev.vendorID << 16 | dev.productID) {
					case 0x54C << 16 | 0x5C4:
					case 0x54C << 16 | 0x9CC:
						dev.flags |= DeviceFlag::SPECIFIC;
						break;
					default:
						break;
					}
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
		using namespace srk::extensions;

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
		GamepadKeyMapper keyMapper;
		auto definedKeyMapper = false;
		switch (di->vendorID << 16 | di->productID) {
		case 0x54C << 16 | 0x5C4:
		case 0x54C << 16 | 0x9CC:
			device = new GenericGamepad(*di, *new GamepadDriverDS4(*this, *hid));
			break;
		default:
			break;
		}

		if (!device) device = new GenericGamepad(*di, *new GamepadDriver(*this, *hid), definedKeyMapper ? &keyMapper : nullptr);

		return device;
	}
}