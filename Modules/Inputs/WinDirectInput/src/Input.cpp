#include "Input.h"
#include "Gamepad.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "CreateModule.h"
#include <algorithm>

namespace aurora::modules::inputs::win_direct_input {
	Input::Input(Ref* loader, Application* app) :
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
		if (!_di && FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&_di, nullptr))) return;

		_di->EnumDevices(DI8DEVCLASS_ALL, _enumDevicesCallback, this, DIEDFL_ATTACHEDONLY);

		std::vector<DeviceInfo> changed;
		if (_keepDevices.size() < _devices.size()) {
			uint32_t size = _keepDevices.size();
			if (size == 0) {
				for (auto& e : _devices) changed.emplace_back(std::move(e));
				_devices.clear();
			} else {
				std::sort(_keepDevices.begin(), _keepDevices.end());

				uint32_t i = size - 1, idx = _devices.size() - 1;
				do {
					do {
						if (idx > i) {
							changed.emplace_back(std::move(_devices[idx]));
							_devices.erase(_devices.begin() + idx);
							--idx;
						} else {
							--idx;
							break;
						}
					} while (true);

					if (i-- == 0) break;
				} while (true);
			}
		}
		_keepDevices.clear();

		uint32_t connectedIdx = changed.size();
		if (_connectedDevices.size()) {
			for (auto& e : _connectedDevices) {
				_devices.emplace_back(e);
				changed.emplace_back(std::move(e));
			}
			_connectedDevices.clear();
		}

		for (uint32_t i = 0; i < connectedIdx; ++i) _eventDispatcher.dispatchEvent(this, ModuleEvent::DISCONNECTED, &changed[i]);
		for (uint32_t i = connectedIdx, n = changed.size(); i < n; ++i) _eventDispatcher.dispatchEvent(this, ModuleEvent::CONNECTED, &changed[i]);
	}

	IInputDevice* Input::createDevice(const DeviceGUID& guid) {
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
		return _app->Win_getHWnd();
	}

	BOOL Input::_enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext) {
		uint32_t type = pdidInstance->dwDevType & 0xFF;
		if (type == DI8DEVTYPE_MOUSE || type == DI8DEVTYPE_KEYBOARD || type == DI8DEVTYPE_GAMEPAD) {
			auto im = (Input*)pContext;

			for (uint32_t i = 0, n = im->_devices.size(); i < n; ++i) {
				if (im->_devices[i].guid.isEqual<false, true>((const uint8_t*)&pdidInstance->guidProduct, sizeof(::GUID))) {
					im->_keepDevices.emplace_back(i);
					return DIENUM_CONTINUE;
				}
			}

			auto& info = im->_connectedDevices.emplace_back();
			info.guid.set<false, true>((const uint8_t*)&pdidInstance->guidProduct, sizeof(::GUID));
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