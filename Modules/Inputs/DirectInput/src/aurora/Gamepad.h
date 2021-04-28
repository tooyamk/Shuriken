#pragma once

#include "DeviceBase.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_BUTTONS = 32;
		static constexpr auto MAX_AXIS_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_1 + (MAX_AXES - 1));
		static constexpr auto MAX_BUTTON_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));


		mutable std::shared_mutex _mutex;
		DIJOYSTATE _state;
		GamepadKeyMapping _keyMapping;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadVirtualKeyCode, Vec2<DeviceStateValue>> _deadZone;

		bool AE_CALL _readState(DIJOYSTATE& state);

		inline Vec2<DeviceStateValue> AE_CALL _getDeadZone(GamepadVirtualKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Vec2<DeviceStateValue>::ZERO;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setKeyMapping(const GamepadKeyMapping* mapping);
		void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone);

		//bool AE_CALL _checkInvalidData(const DIJOYSTATE& state);

		inline static LONG AE_CALL _readAxisVal(const LONG* axes, GamepadKeyCode k, LONG defaultVal) {
			using namespace aurora::enum_operators;

			return k >= GamepadKeyCode::AXIS_1 && k <= MAX_AXIS_KEY ? axes[(uint32_t)(k - GamepadKeyCode::AXIS_1)] : defaultVal;
		}

		inline static BYTE AE_CALL _readButtonVal(const BYTE* buttons, GamepadKeyCode k) {
			using namespace aurora::enum_operators;

			return k >= GamepadKeyCode::BUTTON_1 && k <= MAX_BUTTON_KEY ? buttons[(uint32_t)(k - GamepadKeyCode::BUTTON_1)] : 0;
		}

		DeviceState::CodeType AE_CALL _getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const;
		DeviceState::CodeType AE_CALL _getCombinedTrigger(LONG t, GamepadVirtualKeyCode key, uint8_t index, DeviceStateValue& data) const;
		DeviceState::CodeType AE_CALL _getSeparateTrigger(LONG t, GamepadVirtualKeyCode key, DeviceStateValue& data) const;

		void AE_CALL _dispatchStick(LONG oldX, LONG oldY, LONG newX, LONG newY, GamepadVirtualKeyCode key) const;
		void AE_CALL _dispatchCombinedTrigger(LONG oldVal, LONG newVal) const;
		void AE_CALL _dispatchAxis(LONG oldVal, LONG newVal, GamepadVirtualKeyCode key) const;

		inline static DeviceStateValue AE_CALL _translateDpad(DWORD value) {
			return (value == (std::numeric_limits<DWORD>::max)()) ? Math::NEGATIVE_ONE<DeviceStateValue> : Math::rad(DeviceStateValue(value) * Math::HUNDREDTH<DeviceStateValue>);
		}

		inline static DeviceStateValue AE_CALL _translateButton(DWORD value) {
			return value & 0x80 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		}

		inline static DeviceStateValue AE_CALL _normalizeAxis(LONG value) {
			return DeviceStateValue(value) * Math::RECIPROCAL<DeviceStateValue((std::numeric_limits<uint16_t>::max)())>;
		}

		inline static DeviceStateValue AE_CALL _normalizeStick(LONG value) {
			using T = int32_t;

			constexpr auto MID_LOW = (T)(std::numeric_limits<int16_t>::max)();
			constexpr auto MID_HIGT = MID_LOW + Math::ONE<T>;

			auto v = (T)value - MID_LOW;
			if (v < Math::ZERO<T>) {
				return DeviceStateValue(v) * Math::RECIPROCAL<DeviceStateValue(MID_LOW)>;
			} else if (v > Math::ZERO<T>) {
				return DeviceStateValue(v) * Math::RECIPROCAL<DeviceStateValue(MID_HIGT)>;
			}

			return Math::ZERO<DeviceStateValue>;
		}

		inline static void AE_CALL _normalizeCombinedAxis(LONG value, DeviceStateValue& low, DeviceStateValue& high) {
			constexpr auto MID_LOW = (LONG)((std::numeric_limits<uint16_t>::max)() >> 1);
			constexpr auto MID_HIGT = MID_LOW + Math::ONE<LONG>;

			if (value < MID_LOW) {
				low = DeviceStateValue(MID_LOW - value) * Math::RECIPROCAL<DeviceStateValue(MID_LOW)>;
				high = Math::ZERO<DeviceStateValue>;
			} else if (value > MID_LOW) {
				low = Math::ZERO<DeviceStateValue>;
				high = DeviceStateValue(value - MID_LOW) * Math::RECIPROCAL<DeviceStateValue(MID_HIGT)>;
			} else {
				low = Math::ZERO<DeviceStateValue>;
				high = Math::ZERO<DeviceStateValue>;
			}
		}
	};
}