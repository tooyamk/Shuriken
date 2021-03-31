#include "Input.h"
#include "Gamepad.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "CreateModule.h"
#include <algorithm>

namespace aurora::modules::inputs::direct_input {
	Input::Input(Ref* loader, IApplication* app) :
		_loader(loader),
		_app(app),
		_di(nullptr) {
	}

	Input::~Input() {
		if (_di) _di->Release();
	}

	events::IEventDispatcher<ModuleEvent>& Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		if (!_di && FAILED(DirectInput8Create((HINSTANCE)_app->getNative(ApplicationNative::HINSTANCE), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&_di, nullptr))) return;

		std::vector<DeviceInfo> newDevices;

		_di->EnumDevices(DI8DEVCLASS_ALL, _enumDevicesCallback, &newDevices, DIEDFL_ATTACHEDONLY);

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

		for (auto& info : remove) _eventDispatcher.dispatchEvent(this, ModuleEvent::DISCONNECTED, &info);
		for (auto& info : add) _eventDispatcher.dispatchEvent(this, ModuleEvent::CONNECTED, &info);
	}

	IntrusivePtr<IInputDevice> Input::createDevice(const DeviceGUID& guid) {
		std::shared_lock lock(_mutex);

		for (auto& info : _devices) {
			if (info.guid == guid) {
				LPDIRECTINPUTDEVICE8 dev = nullptr;
				if (FAILED(_di->CreateDevice(*(const ::GUID*)guid.getData(), &dev, nullptr))) return nullptr;

				switch (info.type) {
				case DeviceType::GAMEPAD:
					return new Gamepad(*this, dev, info);
				case DeviceType::KEYBOARD:
					return new Keyboard(*this, dev, info);
				case DeviceType::MOUSE:
					return new Mouse(*this, dev, info);
				default:
					dev->Release();
					return nullptr;
				}
			}
		}

		return nullptr;
	}

	HWND Input::getHWND() const {
		return (HWND)_app->getNative(ApplicationNative::HWND);
	}

	BOOL Input::_enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext) {
		using namespace aurora::enum_operators;

		uint32_t type = pdidInstance->dwDevType & 0xFF;
		if (type == DI8DEVTYPE_MOUSE || type == DI8DEVTYPE_KEYBOARD || type == DI8DEVTYPE_GAMEPAD) {
			auto newDevices = (std::vector<DeviceInfo>*)pContext;

			auto& info = newDevices->emplace_back();
			info.guid.set<false, true>(&pdidInstance->guidProduct, sizeof(::GUID));
			switch (type) {
			case DI8DEVTYPE_MOUSE:
				info.type |= DeviceType::MOUSE;
				break;
			case DI8DEVTYPE_KEYBOARD:
				info.type |= DeviceType::KEYBOARD;
				break;
			case DI8DEVTYPE_GAMEPAD:
				info.type |= DeviceType::GAMEPAD;
				break;
			default:
				break;
			}
		}

		return DIENUM_CONTINUE;
	}
}