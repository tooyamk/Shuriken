#include "Input.h"
#include "Gamepad.h"
#include "CreateModule.h"
#include "base/Application.h"
#include "base/ByteArray.h"
#include <algorithm>

namespace aurora::modules::win_xinput {
	Input::Input(Application* app) :
		_app(app) {
	}

	Input::~Input() {
	}

	events::IEventDispatcher<InputModuleEvent>& Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		GUID guid;

		XINPUT_CAPABILITIES caps;
		for (ui32 i = 0; i < XUSER_MAX_COUNT; ++i) {
			if (XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS) {
				guid.index = i + 1;

				XINPUT_STATE state;
				if (XInputGetState(i, &state) == ERROR_SUCCESS) {
					bool found = false;
					for (ui32 j = 0, n = _devices.size(); j < n; ++j) {
						if (_devices[j].guid.isEqual((const i8*)&guid, sizeof(guid))) {
							_keepDevices.emplace_back(j);
							found = true;
							break;
						}
					}

					if (!found) {
						auto& info = _connectedDevices.emplace_back();
						info.guid.set((const i8*)&guid, sizeof(guid));
						info.type = InputDeviceType::GAMEPAD;
					}
				}
			}
		}

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

	IInputDevice* Input::createDevice(const InputDeviceGUID& guid) {
		for (auto& info : _devices) {
			if (info.guid == guid) return new Gamepad(this, info);
		}

		return nullptr;
	}
}