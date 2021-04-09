#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL GamepadDS4 : public GamepadBase<18, 24> {
	public:
		GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual Key::CountType AE_CALL getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const override;

	protected:
		template<Arithmetic T> inline static constexpr T NUMBER_127 = T(127);
		template<Arithmetic T> inline static constexpr T NUMBER_128 = T(128);
		template<Arithmetic T> inline static constexpr T NUMBER_255 = T(255);

		enum class InputBufferOffset : uint8_t {
			LX = 0,
			LY,
			RX,
			RY,
			D_PAD = 4,
			SQUARE = 4,
			CROSS = 4,
			CIRCLE = 4,
			TRIANGLE = 4,
			L1 = 5,
			R1 = 5,
			L2 = 5,
			R2 = 5,
			SHARE = 5,
			OPTIONS = 5,
			L3 = 5,
			R3 = 5,
			PS = 6,
			TOUTCH_PAD_CLICK = 6,
			COUNTER = 6,
			L_TRIGGER = 7,
			R_TRIGGER,
			BATTERY = 11,
			BATTERY_LEVEL = 29,
			TOUCHES = 32,
			PACKET_COUNTER = 33,
			FINGER1 = 34,
			FINGER2 = 38
		};

		enum class InputMask : uint8_t {
			NONE = 0,

			SQUARE = 0b10000,
			CROSS = 0b100000,
			CIRCLE = 0b1000000,
			TRIANGLE = 0b10000000,

			L1 = 0b1,
			R1 = 0b10,
			L2 = 0b100,
			R2 = 0b1000,
			SHARE = 0b10000,
			OPTIONS = 0b100000,
			L3 = 0b1000000,
			R3 = 0b10000000,

			PS = 0b1,
			TOUTCH_PAD_CLICK = 0b10
		};


		virtual void AE_CALL _parse(bool dispatchEvent, ReadBuffer& readBuffer, size_t readBufferSize) override;

		void AE_CALL _dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(uint8_t oldState, uint8_t newState, GamepadKeyCode key);
		void AE_CALL _dispatchDPad(uint8_t oldState, uint8_t newState);
		void AE_CALL _dispatchStick(uint16_t oldState, uint16_t newState, GamepadKeyCode key);

		inline static Key::ValueType AE_CALL _translateTrigger(uint8_t value) {
			return Key::ValueType(value) * Math::RECIPROCAL<NUMBER_255<Key::ValueType>>;
		}

		inline static Key::ValueType AE_CALL _translateStick(uint8_t value) {
			if (value < NUMBER_127<decltype(value)>) {
				return (Key::ValueType)(value - NUMBER_127<decltype(value)>) * Math::RECIPROCAL<NUMBER_127<Key::ValueType>>;
			} else if (value > 127) {
				return (Key::ValueType)(value - NUMBER_127<decltype(value)>) * Math::RECIPROCAL<NUMBER_128<Key::ValueType>>;
			} else {
				return Math::ZERO<Key::ValueType>;
			}
		}
	};
}