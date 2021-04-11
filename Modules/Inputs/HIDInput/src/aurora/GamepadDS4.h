#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL GamepadDS4 : public GamepadBase<8, 10, 11> {
	public:
		GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) override;

	protected:
		using OutputBuffer = uint8_t[5];

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
			TOUTCH_PAD = 6,
			COUNTER = 6,
			L_TRIGGER = 7,
			R_TRIGGER,
			BATTERY = 11,
			BATTERY_LEVEL = 29,
			TOUCHES = 32,
			TOUCH_PACKET_COUNTER = 33,
			TOUCHE1 = 34,
			TOUCHE2 = 38
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


		bool _isBluetooth;

		mutable std::shared_mutex _inputStateMutex;

		mutable std::shared_mutex _outputBufferMutex;
		std::atomic_bool _outputDirty;
		uint8_t _outputOffset;
		OutputBuffer _outputBuffer;

		virtual void AE_CALL _doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) override;
		virtual bool AE_CALL _doOutput() override;

		void AE_CALL _setVibration(DeviceState::ValueType left, DeviceState::ValueType right);
		void AE_CALL _setLed(DeviceState::ValueType red, DeviceState::ValueType green, DeviceState::ValueType blue);

		DeviceState::CountType AE_CALL _getButton(uint8_t state, InputMask mask, DeviceState::ValueType* data) const;
		DeviceState::CountType AE_CALL _getTrigger(uint8_t state, GamepadKeyCode key, DeviceState::ValueType* data) const;
		DeviceState::CountType AE_CALL _getDPad(uint8_t state, DeviceState::ValueType* data) const;
		DeviceState::CountType AE_CALL _getStick(uint8_t xstate, uint8_t ystate, GamepadKeyCode key, DeviceState::ValueType* data, DeviceState::CountType count) const;

		void AE_CALL _dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(uint8_t oldState, uint8_t newState, GamepadKeyCode key);
		void AE_CALL _dispatchDPad(uint8_t oldState, uint8_t newState);
		void AE_CALL _dispatchStick(uint16_t oldState, uint16_t newState, GamepadKeyCode key);

		inline static DeviceState::ValueType AE_CALL _translateTrigger(uint8_t value) {
			return DeviceState::ValueType(value) * Math::RECIPROCAL<NUMBER_255<DeviceState::ValueType>>;
		}

		inline static DeviceState::ValueType AE_CALL _translateStick(uint8_t value) {
			if (value < NUMBER_127<decltype(value)>) {
				return (DeviceState::ValueType)(value - NUMBER_127<decltype(value)>) * Math::RECIPROCAL<NUMBER_127<DeviceState::ValueType>>;
			} else if (value > 127) {
				return (DeviceState::ValueType)(value - NUMBER_127<decltype(value)>) * Math::RECIPROCAL<NUMBER_128<DeviceState::ValueType>>;
			} else {
				return Math::ZERO<DeviceState::ValueType>;
			}
		}
	};
}