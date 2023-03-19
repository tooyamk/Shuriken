#include "Input.h"
#include "GamepadDriver.h"
#include "CreateModule.h"
#include "srk/DynamicLibraryLoader.h"

namespace srk::modules::inputs::xinput {
	Input::Input(Ref* loader, const CreateInputModuleDesc& desc) :
		_loader(loader),
		_filters(desc.filters),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()),
		_XInputGetCapabilitiesEx(nullptr) {
		auto useHiddenAPI1_4 = false;
		for (size_t i = 1; i < desc.argc; i += 2) {
			std::string_view key = (const char*)desc.argv[i - 1];
			if (key == "use-hidden-api-1-4") {
				useHiddenAPI1_4 = *(bool*)desc.argv[i];
			}
		}
		if (useHiddenAPI1_4 && _xinputDll.load("XInput1_4.dll")) _XInputGetCapabilitiesEx = (XInputGetCapabilitiesEx)_xinputDll.getSymbolAddress(std::string_view((char*)108, 1));
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace srk::enum_operators;

		if ((DeviceType::GAMEPAD & _filters) == DeviceType::UNKNOWN) return;

		std::vector<DeviceInfo> newDevices;

		InternalGUID guid;

		XINPUT_STATE state;
		XINPUT_CAPABILITIES_EX capsEx;
		for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i) {
			guid.index = i + 1;

			if (XInputGetState(i, &state) == ERROR_SUCCESS) {
				bool found = false;

				auto& info = newDevices.emplace_back();
				info.type = DeviceType::GAMEPAD;

				if (_XInputGetCapabilitiesEx && _XInputGetCapabilitiesEx(1, i, 0, &capsEx) == ERROR_SUCCESS) {
					info.vendorID = capsEx.vendorId;
					info.productID = capsEx.productId;

					guid.vendorID = info.vendorID;
					guid.productID = info.productID;
				} else {
					guid.vendorID = 0;
					guid.productID = 0;
				}

				info.guid.set<false, true>(&guid, sizeof(guid));
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
		std::shared_lock lock(_mutex);

		for (auto& info : _devices) {
			if (info.guid == guid) {
				XINPUT_CAPABILITIES_EX capsEx;
				uint16_t vendorID = 0, productID = 0;
				if (_XInputGetCapabilitiesEx && _XInputGetCapabilitiesEx(1, ((InternalGUID&)*info.guid.getData()).index - 1, 0, &capsEx) == ERROR_SUCCESS) {
					vendorID = capsEx.vendorId;
					productID = capsEx.productId;
				}
				if (info.vendorID != vendorID || info.productID != productID) return nullptr;

				return new GenericGamepad(info, *new GamepadDriver(*this, info));
			}
		}

		return nullptr;
	}
}