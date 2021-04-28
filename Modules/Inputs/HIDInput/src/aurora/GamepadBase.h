#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t InputStateBufferSize, size_t InputBufferSize, size_t OutputStateBufferSize>
	class GamepadBase : public DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize> {
	public:
		GamepadBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : 
			DeviceBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize>(input, info, hid) {
			Vec2<DeviceStateValue> dz(Math::ZERO<DeviceStateValue>, Math::TWENTIETH<DeviceStateValue>);
			_setDeadZone(GamepadVirtualKeyCode::L_STICK, &dz);
			_setDeadZone(GamepadVirtualKeyCode::R_STICK, &dz);
			_setDeadZone(GamepadVirtualKeyCode::L_TRIGGER, &dz);
			_setDeadZone(GamepadVirtualKeyCode::R_TRIGGER, &dz);
		}

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override {
			switch (type) {
			case DeviceStateType::DEAD_ZONE:
			{
				if (values && count) {
					DeviceState::CountType c = 1;

					auto dz = _getDeadZone((GamepadVirtualKeyCode)code);
					((DeviceStateValue*)values)[0] = dz[0];
					if (count > 1) ((DeviceStateValue*)values)[c++] = dz[1];

					return c;
				}

				return 0;
			}
			default:
				return 0;
			}
		}
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override {
			switch (type) {
			case DeviceStateType::DEAD_ZONE:
			{
				if (!count) values = nullptr;

				if (values) {
					DeviceState::CountType c = 1;

					Vec2<DeviceStateValue> dz;
					dz[0] = ((DeviceStateValue*)values)[0];
					if (count > 1) dz[c++] = ((DeviceStateValue*)values)[1];

					_setDeadZone((GamepadVirtualKeyCode)code, &dz);

					return c;
				} else {
					_setDeadZone((GamepadVirtualKeyCode)code, nullptr);
					return 1;
				}

				return 0;
			}
			default:
				return 0;
			}
		}

	protected:
		GamepadKeyMapping _keyMapping;
		using GamepadBaseType = GamepadBase<InputStateBufferSize, InputBufferSize, OutputStateBufferSize>;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadVirtualKeyCode, Vec2<DeviceStateValue>> _deadZone;

		inline Vec2<DeviceStateValue> AE_CALL _getDeadZone(GamepadVirtualKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Vec2<DeviceStateValue>::ZERO;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone) {
			if (deadZone) {
				auto& dzVal = *deadZone;

				Math::clamp(dzVal.data, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>);

				if (dzVal[1] < dzVal[0]) {
					auto tmp = dzVal[0];
					dzVal[0] = dzVal[1];
					dzVal[1] = tmp;
				}

				std::scoped_lock lock(_deadZoneMutex);

				_deadZone.insert_or_assign(keyCode, dzVal);
			} else {
				std::scoped_lock lock(_deadZoneMutex);

				if (auto itr = _deadZone.find(keyCode); itr != _deadZone.end()) _deadZone.erase(itr);
			}
		}
	};
}