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
		template<Arithmetic T> inline static constexpr T NUMBER_32767 = T(32767);
		template<Arithmetic T> inline static constexpr T NUMBER_32768 = T(32768);
		template<Arithmetic T> inline static constexpr T NUMBER_65535 = T(65535);

		struct KeyMapping {
			uint8_t LSTICK_X;
			uint8_t LSTICK_Y;
			uint8_t RSTICK_X;
			uint8_t RSTICK_Y;
			uint8_t LTRIGGER;
			uint8_t RTRIGGER;

			std::unordered_map<uint8_t, GamepadKeyCode> BUTTONS;
		};


		mutable std::shared_mutex _mutex;
		DIJOYSTATE2 _state;
		const KeyMapping* _keyMapping;
		std::unordered_map<GamepadKeyCode, uint8_t> _enumToKeyMapping;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadKeyCode, DeviceStateValue> _deadZone;

		inline DeviceStateValue AE_CALL _getDeadZone(GamepadKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Math::ZERO<DeviceStateValue>;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setDeadZone(GamepadKeyCode keyCode, DeviceStateValue deadZone);

		bool AE_CALL _checkInvalidData(const DIJOYSTATE2& state);

		DeviceState::CodeType AE_CALL _getStick(LONG x, LONG y, GamepadKeyCode key, DeviceStateValue* data, uint32_t count) const;
		DeviceState::CodeType AE_CALL _getTrigger(LONG t, GamepadKeyCode key, uint8_t index, DeviceStateValue& data) const;
		DeviceState::CodeType AE_CALL _getTriggerSeparate(LONG t, GamepadKeyCode key, DeviceStateValue& data) const;

		void AE_CALL _dispatchStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey);
		void AE_CALL _dispatchTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key);

		inline static DeviceStateValue AE_CALL _translateDeadZone01(DeviceStateValue value, DeviceStateValue dz, bool inDz) {
			return inDz ? Math::ZERO<DeviceStateValue> : (value - dz) / (Math::ONE<DeviceStateValue> - dz);
		}

		static DeviceStateValue AE_CALL _translateStick(LONG value);
		static void AE_CALL _translateTrigger(LONG value, DeviceStateValue& l, DeviceStateValue& r);
		inline static DeviceStateValue AE_CALL _translateTriggerSeparate(LONG value) {
			return DeviceStateValue(value) / NUMBER_65535<DeviceStateValue>;
		}
		inline static DeviceStateValue AE_CALL _translateDpad(DWORD value) {
			return (value == (std::numeric_limits<DWORD>::max)()) ? Math::NEGATIVE_ONE<DeviceStateValue> : Math::rad(DeviceStateValue(value) * Math::HUNDREDTH<DeviceStateValue>);
		}
		inline static DeviceStateValue AE_CALL _translateButton(DWORD value) {
			return value & 0x80 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		}

		static const KeyMapping DIRECT;
		static const KeyMapping XINPUT;
		static const KeyMapping DS4;
	};
}