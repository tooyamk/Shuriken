#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	class GamepadBase : public DeviceBase {
	public:
		GamepadBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;

	protected:
		GamepadKeyMapping _keyMapping;

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

		void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone);
	};
}