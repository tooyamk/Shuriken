#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t InputStateBufferSize, size_t InputBufferSize, size_t OutputStateBufferSize>
	class GamepadBase : public DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize> {
	public:
		GamepadBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : 
			DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize>(input, info, hid) {
			_setDeadZone(GamepadKeyCode::L_STICK, Math::TWENTIETH<DeviceState::ValueType>);
			_setDeadZone(GamepadKeyCode::R_STICK, Math::TWENTIETH<DeviceState::ValueType>);
			_setDeadZone(GamepadKeyCode::L_TRIGGER, Math::TWENTIETH<DeviceState::ValueType>);
			_setDeadZone(GamepadKeyCode::R_TRIGGER, Math::TWENTIETH<DeviceState::ValueType>);
		}

	protected:
		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadKeyCode, DeviceState::ValueType> _deadZone;

		inline DeviceState::ValueType AE_CALL _getDeadZone(GamepadKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Math::ZERO<DeviceState::ValueType>;
			} else {
				return itr->second;
			}
		}

		inline void AE_CALL _setDeadZone(GamepadKeyCode keyCode, DeviceState::ValueType deadZone) {
			if (deadZone < Math::ZERO<DeviceState::ValueType>) deadZone = -deadZone;

			std::scoped_lock lock(_deadZoneMutex);

			_deadZone.insert_or_assign(keyCode, deadZone);
		}

		inline static DeviceState::ValueType AE_CALL _translateDeadZone01(DeviceState::ValueType value, DeviceState::ValueType dz, bool inDz) {
			return inDz ? Math::ZERO<DeviceState::ValueType> : (value - dz) / (Math::ONE<DeviceState::ValueType> - dz);
		}
	};
}