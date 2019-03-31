#include "DirectInput.h"
#include "Keyboard.h"
#include <algorithm>

namespace aurora::modules::win_direct_input {
	DirectInput::DirectInput() :
		_di(nullptr) {
	}

	DirectInput::~DirectInput() {
		if (_di) _di->Release();
	}

	events::IEventDispatcher<InputModuleEvent>& DirectInput::getEventDispatcher() {
		return _eventDispatcher;
	}

	void DirectInput::poll() {
		if (!_di && FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&_di, nullptr))) return;

		_di->EnumDevices(DI8DEVCLASS_ALL, _enumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
		
		std::vector<InputDeviceInfo> changed;
		if (_keepDevices.size() < _devices.size()) {
			ui32 size = _keepDevices.size();
			if (size == 0) {
				for (auto& e : _devices) changed.emplace_back(std::move(e));
				_devices.clear();
			} else {
				std::sort(_keepDevices.begin(), _keepDevices.end());

				ui32 i = size - 1, idx = _devices.size() - 1;
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

		ui32 connectedIdx = changed.size();
		if (_connectedDevices.size() > 0) {
			for (auto& e : _connectedDevices) {
				_devices.emplace_back(e);
				changed.emplace_back(std::move(e));
			}
			_connectedDevices.clear();
		}

		for (ui32 i = 0; i < connectedIdx; ++i) _eventDispatcher.dispatchEvent(this, InputModuleEvent::DISCONNECTED, &changed[i]);
		for (ui32 i = connectedIdx, n = changed.size(); i < n; ++i) _eventDispatcher.dispatchEvent(this, InputModuleEvent::CONNECTED, &changed[i]);
	}

	InputDevice* DirectInput::createDevice(const InputDeviceGUID& guid) const {
		for (auto& info : _devices) {
			if (info.guid == guid) {
				LPDIRECTINPUTDEVICE8 dev = nullptr;
				if (FAILED(_di->CreateDevice(*(const GUID*)guid.getData(), &dev, nullptr))) return nullptr;


				switch (info.type) {
				case InputDeviceType::KEYBOARD:
					return new Keyboard(guid);
				default:
					return nullptr;
				}
			}
		}

		return nullptr;
	}

	BOOL DirectInput::_enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext) {
		ui32 type = pdidInstance->dwDevType & 0xFF;
		if (type == DI8DEVTYPE_MOUSE || type == DI8DEVTYPE_KEYBOARD || type == DI8DEVTYPE_JOYSTICK) {
			auto im = (DirectInput*)pContext;

			for (ui32 i = 0, n = im->_devices.size(); i < n; ++i) {
				if (im->_devices[i].guid.isEqual((ui8*)&pdidInstance->guidInstance, sizeof(GUID))) {
					im->_keepDevices.emplace_back(i);
					return DIENUM_CONTINUE;
				}
			}

			auto& info = im->_connectedDevices.emplace_back();
			info.guid.set((ui8*)&pdidInstance->guidInstance, sizeof(GUID));
			switch (type) {
			case DI8DEVTYPE_MOUSE:
				info.type |= InputDeviceType::MOUSE;
				break;
			case DI8DEVTYPE_KEYBOARD:
				info.type |= InputDeviceType::KEYBOARD;
				break;
			case DI8DEVTYPE_JOYSTICK:
				info.type |= InputDeviceType::GAMEPAD;
				break;
			default:
				break;
			}
		}

		return DIENUM_CONTINUE;
	}
}