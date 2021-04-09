#pragma once

#include "DeviceBase.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info);

		virtual Key::CountType AE_CALL getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(Key::CodeType keyCode, Key::ValueType deadZone) override;

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
		std::unordered_map<Key::CodeType, Key::ValueType> _deadZone;

		inline Key::ValueType AE_CALL _getDeadZone(GamepadKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find((Key::CodeType)key); itr == _deadZone.end()) {
				return Math::ZERO<Key::ValueType>;
			} else {
				return itr->second;
			}
		}

		bool AE_CALL _checkInvalidData(const DIJOYSTATE2& state);

		Key::CodeType AE_CALL _getStick(LONG x, LONG y, GamepadKeyCode key, Key::ValueType* data, uint32_t count) const;
		Key::CodeType AE_CALL _getTrigger(LONG t, GamepadKeyCode key, uint8_t index, Key::ValueType& data) const;
		Key::CodeType AE_CALL _getTriggerSeparate(LONG t, GamepadKeyCode key, Key::ValueType& data) const;

		void AE_CALL _dispatchStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey);
		void AE_CALL _dispatchTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key);

		inline static Key::ValueType AE_CALL _translateDeadZone01(Key::ValueType value, Key::ValueType dz, bool inDz) {
			return inDz ? Math::ZERO<Key::ValueType> : (value - dz) / (Math::ONE<Key::ValueType> - dz);
		}

		static Key::ValueType AE_CALL _translateStick(LONG value);
		static void AE_CALL _translateTrigger(LONG value, Key::ValueType& l, Key::ValueType& r);
		inline static Key::ValueType AE_CALL _translateTriggerSeparate(LONG value) {
			return Key::ValueType(value) / NUMBER_65535<Key::ValueType>;
		}
		inline static Key::ValueType AE_CALL _translateDpad(DWORD value) {
			return (value == (std::numeric_limits<DWORD>::max)()) ? Math::NEGATIVE_ONE<Key::ValueType> : Math::rad(Key::ValueType(value) * Math::HUNDREDTH<Key::ValueType>);
		}
		inline static Key::ValueType AE_CALL _translateButton(DWORD value) {
			return value & 0x80 ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
		}

		static const KeyMapping DIRECT;
		static const KeyMapping XINPUT;
		static const KeyMapping DS4;
	};
}