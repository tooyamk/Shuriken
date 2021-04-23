#pragma once

#include "DeviceBase.h"
#include "aurora/SerializableObject.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		static constexpr size_t MAX_BUTTONS = 32;

		Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		template<Arithmetic T> inline static constexpr T NUMBER_32767 = T(32767);
		template<Arithmetic T> inline static constexpr T NUMBER_32768 = T(32768);
		template<Arithmetic T> inline static constexpr T NUMBER_65535 = T(65535);


		struct KeyMapping {
			union {
				struct {
					uint8_t lStick[2];
					uint8_t rStick[2];
					uint8_t trigger[2];
				};

				uint8_t axis[6];
			};
			

			GamepadVirtualKeyCode buttons[MAX_BUTTONS];
		};


		mutable std::shared_mutex _mutex;
		DIJOYSTATE _state;
		KeyMapping _keyMapping;
		std::unordered_map<GamepadVirtualKeyCode, uint8_t> _enumToKeyMapping;

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

		void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, DeviceStateValue deadZone);

		bool AE_CALL _checkInvalidData(const DIJOYSTATE& state);

		DeviceState::CodeType AE_CALL _getStick(LONG x, LONG y, GamepadVirtualKeyCode key, DeviceStateValue* data, uint32_t count) const;
		DeviceState::CodeType AE_CALL _getTrigger(LONG t, GamepadVirtualKeyCode key, uint8_t index, DeviceStateValue& data) const;
		DeviceState::CodeType AE_CALL _getTriggerSeparate(LONG t, GamepadVirtualKeyCode key, DeviceStateValue& data) const;

		void AE_CALL _dispatchStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchTrigger(LONG ori, LONG cur, GamepadVirtualKeyCode lkey, GamepadVirtualKeyCode rkey);
		void AE_CALL _dispatchTriggerSeparate(LONG ori, LONG cur, GamepadVirtualKeyCode key);

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

		void AE_CALL _setKeyMapping(const SerializableObject* mapping);

		template<GamepadVirtualKeyCode... Buttons>
		static Gamepad::KeyMapping _createKeyMapping(GamepadKeyCode lstickX, GamepadKeyCode lstickY, GamepadKeyCode rstickX, GamepadKeyCode rstickY, GamepadKeyCode ltrigger, GamepadKeyCode rtrigger) {
			Gamepad::KeyMapping mapping;
			mapping.lStick[0] = (uint8_t)lstickX;
			mapping.lStick[1] = (uint8_t)lstickY;
			mapping.rStick[0] = (uint8_t)rstickX;
			mapping.rStick[1] = (uint8_t)rstickY;
			mapping.trigger[0] = (uint8_t)ltrigger;
			mapping.trigger[1] = (uint8_t)rtrigger;

			size_t i = 0;
			auto btns = mapping.buttons;
			((btns[i++] = Buttons), ...);
			memset(btns + sizeof...(Buttons), (uint8_t)GamepadVirtualKeyCode::UNDEFINED_BUTTON, sizeof(GamepadVirtualKeyCode) * (Gamepad::MAX_BUTTONS - sizeof...(Buttons)));

			return mapping;
		}

		static const KeyMapping DIRECT;
		static const KeyMapping XINPUT;
		static const KeyMapping DS4;
	};
}