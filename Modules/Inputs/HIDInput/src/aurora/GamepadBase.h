#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t InputStateBufferSize, size_t InputBufferSize, size_t OutputStateBufferSize>
	class GamepadBase : public DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize> {
	public:
		GamepadBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : 
			DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize>(input, info, hid) {
			_setDeadZone(GamepadVirtualKeyCode::L_STICK, Math::TWENTIETH<DeviceStateValue>);
			_setDeadZone(GamepadVirtualKeyCode::R_STICK, Math::TWENTIETH<DeviceStateValue>);
			_setDeadZone(GamepadVirtualKeyCode::L_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
			_setDeadZone(GamepadVirtualKeyCode::R_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
		}

	protected:
		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadVirtualKeyCode, DeviceStateValue> _deadZone;

		inline DeviceStateValue AE_CALL _getDeadZone(GamepadVirtualKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Math::ZERO<DeviceStateValue>;
			} else {
				return itr->second;
			}
		}

		inline void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, DeviceStateValue deadZone) {
			if (deadZone < Math::ZERO<DeviceStateValue>) deadZone = -deadZone;

			std::scoped_lock lock(_deadZoneMutex);

			_deadZone.insert_or_assign(keyCode, deadZone);
		}

		inline static DeviceStateValue AE_CALL _translateDeadZone01(DeviceStateValue value, DeviceStateValue dz, bool inDz) {
			return inDz ? Math::ZERO<DeviceStateValue> : (value - dz) / (Math::ONE<DeviceStateValue> - dz);
		}
	};
}