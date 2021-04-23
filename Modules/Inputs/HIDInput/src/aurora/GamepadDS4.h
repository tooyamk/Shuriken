#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL GamepadDS4 : public GamepadBase<9, 44, 11> {
	public:
		GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;

	protected:
		using OutputBuffer = uint8_t[5];
		using TouchBuffer = uint8_t[11];

		template<Arithmetic T> inline static constexpr T NUMBER_127 = T(127);
		template<Arithmetic T> inline static constexpr T NUMBER_128 = T(128);
		template<Arithmetic T> inline static constexpr T NUMBER_255 = T(255);

		inline static constexpr DeviceStateValue TOUCH_PAD_RESOLUTION_X = 1920;
		inline static constexpr DeviceStateValue TOUCH_PAD_RESOLUTION_Y = 943;
		inline static constexpr DeviceStateValue TOUCH_PAD_MAX_X = TOUCH_PAD_RESOLUTION_X - Math::ONE<DeviceStateValue>;
		inline static constexpr DeviceStateValue TOUCH_PAD_MAX_Y = TOUCH_PAD_RESOLUTION_Y - Math::ONE<DeviceStateValue>;

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
			BATTERY_LEVEL = 11,
			GYRO_X = 12,
			GYRO_Y = 14,
			GYRO_Z = 16,
			ACCEL_X = 18,
			ACCEL_Y = 20,
			ACCEL_Z = 22,
			//BATTERY_LEVEL = 29,
			TOUCHES = 32,
			TOUCH_PACKET_COUNTER = 33,
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


		bool _isBluetooth;

		mutable std::shared_mutex _inputStateMutex;
		InternalDeviceTouchStateValue _touchStateValues[2];

		mutable std::shared_mutex _outputBufferMutex;
		std::atomic_bool _outputDirty;
		uint8_t _outputOffset;
		OutputBuffer _outputBuffer;

		virtual void AE_CALL _doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) override;
		virtual bool AE_CALL _doOutput() override;

		void AE_CALL _setVibration(DeviceStateValue left, DeviceStateValue right);
		void AE_CALL _setLed(DeviceStateValue red, DeviceStateValue green, DeviceStateValue blue);

		static DeviceState::CountType AE_CALL _getButton(uint8_t state, InputMask mask, DeviceStateValue* data);
		DeviceState::CountType AE_CALL _getTrigger(uint8_t state, GamepadVirtualKeyCode key, DeviceStateValue* data) const;
		static DeviceState::CountType AE_CALL _getDPad(uint8_t state, DeviceStateValue* data);
		DeviceState::CountType AE_CALL _getStick(uint8_t xstate, uint8_t ystate, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const;
		static void AE_CALL _getTouch(const InternalDeviceTouchStateValue& in, size_t time, DeviceTouchStateValue& out);

		void AE_CALL _dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchTrigger(uint8_t oldState, uint8_t newState, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchDPad(uint8_t oldState, uint8_t newState);
		void AE_CALL _dispatchStick(uint16_t oldState, uint16_t newState, GamepadVirtualKeyCode key);

		inline static DeviceStateValue AE_CALL _translateTrigger(uint8_t value) {
			return DeviceStateValue(value) * Math::RECIPROCAL<NUMBER_255<DeviceStateValue>>;
		}

		inline static DeviceStateValue AE_CALL _translateStick(uint8_t value) {
			if (value < NUMBER_127<decltype(value)>) {
				return (DeviceStateValue)(value - NUMBER_127<decltype(value)>) * Math::RECIPROCAL<NUMBER_127<DeviceStateValue>>;
			} else if (value > 127) {
				return (DeviceStateValue)(value - NUMBER_127<decltype(value)>) * Math::RECIPROCAL<NUMBER_128<DeviceStateValue>>;
			} else {
				return Math::ZERO<DeviceStateValue>;
			}
		}

		void static AE_CALL _translateTouch(uint8_t* data, InternalDeviceTouchStateValue* states, size_t time);
	};
}