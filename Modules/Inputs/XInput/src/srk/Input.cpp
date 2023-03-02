#include "Input.h"
#include "GamepadDriver.h"
#include "CreateModule.h"

namespace srk::modules::inputs::xinput {
	Input::Input(Ref* loader, const CreateInputModuleDesc& desc) :
		_loader(loader),
		_filters(desc.filters),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()) {
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
		for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i) {
			guid.index = i + 1;

			if (XInputGetState(i, &state) == ERROR_SUCCESS) {
				bool found = false;

				auto& info = newDevices.emplace_back();
				info.guid.set<false, true>(&guid, sizeof(guid));
				info.type = DeviceType::GAMEPAD;
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
			if (info.guid == guid) return new GenericGamepad(info, *new GamepadDriver(*this, info));
		}

		return nullptr;
	}
}