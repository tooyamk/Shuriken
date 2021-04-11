#pragma once

#include "DeviceBase.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) override;
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
		std::unordered_map<GamepadKeyCode, DeviceState::ValueType> _deadZone;

		inline DeviceState::ValueType AE_CALL _getDeadZone(GamepadKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Math::ZERO<DeviceState::ValueType>;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setDeadZone(GamepadKeyCode keyCode, DeviceState::ValueType deadZone);

		bool AE_CALL _checkInvalidData(const DIJOYSTATE2& state);

		DeviceState::CodeType AE_CALL _getStick(LONG x, LONG y, GamepadKeyCode key, DeviceState::ValueType* data, uint32_t count) const;
		DeviceState::CodeType AE_CALL _getTrigger(LONG t, GamepadKeyCode key, uint8_t index, DeviceState::ValueType& data) const;
		DeviceState::CodeType AE_CALL _getTriggerSeparate(LONG t, GamepadKeyCode key, DeviceState::ValueType& data) const;

		void AE_CALL _dispatchStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey);
		void AE_CALL _dispatchTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key);

		inline static DeviceState::ValueType AE_CALL _translateDeadZone01(DeviceState::ValueType value, DeviceState::ValueType dz, bool inDz) {
			return inDz ? Math::ZERO<DeviceState::ValueType> : (value - dz) / (Math::ONE<DeviceState::ValueType> - dz);
		}

		static DeviceState::ValueType AE_CALL _translateStick(LONG value);
		static void AE_CALL _translateTrigger(LONG value, DeviceState::ValueType& l, DeviceState::ValueType& r);
		inline static DeviceState::ValueType AE_CALL _translateTriggerSeparate(LONG value) {
			return DeviceState::ValueType(value) / NUMBER_65535<DeviceState::ValueType>;
		}
		inline static DeviceState::ValueType AE_CALL _translateDpad(DWORD value) {
			return (value == (std::numeric_limits<DWORD>::max)()) ? Math::NEGATIVE_ONE<DeviceState::ValueType> : Math::rad(DeviceState::ValueType(value) * Math::HUNDREDTH<DeviceState::ValueType>);
		}
		inline static DeviceState::ValueType AE_CALL _translateButton(DWORD value) {
			return value & 0x80 ? Math::ONE<DeviceState::ValueType> : Math::ZERO<DeviceState::ValueType>;
		}

		static const KeyMapping DIRECT;
		static const KeyMapping XINPUT;
		static const KeyMapping DS4;
	};
}