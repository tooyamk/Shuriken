#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL GamepadDS4 : public GamepadBase {
	public:
		GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		using InputBuffer = uint8_t[45];
		using InputState = uint8_t[9];
		using OutputBuffer = uint8_t[9];
		using OutputState = uint8_t[5];
		using TouchBuffer = uint8_t[11];


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


		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_BUTTONS = 14;
		static constexpr uint8_t BUTTON_OFFSET[] = {
			(uint8_t)InputBufferOffset::SQUARE, (uint8_t)InputBufferOffset::CROSS, (uint8_t)InputBufferOffset::CIRCLE, (uint8_t)InputBufferOffset::TRIANGLE,
			(uint8_t)InputBufferOffset::L1, (uint8_t)InputBufferOffset::R1,
			(uint8_t)InputBufferOffset::L2, (uint8_t)InputBufferOffset::R2,
			(uint8_t)InputBufferOffset::SHARE, (uint8_t)InputBufferOffset::OPTIONS,
			(uint8_t)InputBufferOffset::L3, (uint8_t)InputBufferOffset::R3,
			(uint8_t)InputBufferOffset::PS, (uint8_t)InputBufferOffset::TOUTCH_PAD
		};
		static constexpr uint16_t BUTTON_MASK[] = {
			(uint8_t)InputMask::SQUARE, (uint8_t)InputMask::CROSS, (uint8_t)InputMask::CIRCLE, (uint8_t)InputMask::TRIANGLE,
			(uint8_t)InputMask::L1, (uint8_t)InputMask::R1,
			(uint8_t)InputMask::L2, (uint8_t)InputMask::R2,
			(uint8_t)InputMask::SHARE, (uint8_t)InputMask::OPTIONS,
			(uint8_t)InputMask::L3, (uint8_t)InputMask::R3,
			(uint8_t)InputMask::PS, (uint8_t)InputMask::TOUTCH_PAD_CLICK
		};
		static constexpr auto MAX_BUTTON_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));

		template<Arithmetic T> inline static constexpr T NUMBER_127 = T(127);
		template<Arithmetic T> inline static constexpr T NUMBER_128 = T(128);
		template<Arithmetic T> inline static constexpr T NUMBER_255 = T(255);

		inline static constexpr DeviceStateValue TOUCH_PAD_RESOLUTION_X = 1920;
		inline static constexpr DeviceStateValue TOUCH_PAD_RESOLUTION_Y = 943;
		inline static constexpr DeviceStateValue TOUCH_PAD_MAX_X = TOUCH_PAD_RESOLUTION_X - Math::ONE<DeviceStateValue>;
		inline static constexpr DeviceStateValue TOUCH_PAD_MAX_Y = TOUCH_PAD_RESOLUTION_Y - Math::ONE<DeviceStateValue>;


		bool _isBluetooth;
		bool _needOutput;

		mutable std::shared_mutex _inputMutex;
		DeviceTouchStateValue _touchStateValues[2];
		InputState _inputState;

		mutable std::shared_mutex _outputMutex;
		std::atomic_bool _outputDirty;
		uint8_t _inputBufferOffset, _outputBufferOffset;
		OutputBuffer _outputBuffer;
		OutputState _outputState;

		void AE_CALL _doInput(bool dispatchEvent);
		void AE_CALL _doOutput();

		void AE_CALL _setKeyMapping(const GamepadKeyMapping* mapping);
		void AE_CALL _setVibration(DeviceStateValue left, DeviceStateValue right);
		void AE_CALL _setLed(DeviceStateValue red, DeviceStateValue green, DeviceStateValue blue);

		static uint8_t AE_CALL _readAxisVal(const uint8_t* state, GamepadKeyCode k, uint8_t defaultVal);

		inline static uint8_t AE_CALL _readButtonVal(const uint8_t* state, GamepadKeyCode k) {
			using namespace aurora::enum_operators;

			if (k >= GamepadKeyCode::BUTTON_1 && k <= MAX_BUTTON_KEY) {
				auto i = (size_t)(k - GamepadKeyCode::BUTTON_1);
				return state[BUTTON_OFFSET[i]] & BUTTON_MASK[i];
			} else {
				return 0;
			}
		}

		static DeviceState::CountType AE_CALL _getButton(uint8_t state, InputMask mask, DeviceStateValue* data);
		static DeviceState::CountType AE_CALL _getDpad(uint8_t state, DeviceStateValue* data);
		DeviceState::CountType AE_CALL _getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const;

		void AE_CALL _dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchDpad(uint8_t oldState, uint8_t newState);
		void AE_CALL _dispatchStick(uint8_t oldX, uint8_t oldY, uint8_t newX, uint8_t newY, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchAxis(uint8_t oldVal, uint8_t newVal, GamepadVirtualKeyCode key);

		inline static DeviceStateValue AE_CALL _normalizeStick(uint8_t value) {
			using T = int16_t;

			constexpr auto MID_LOW = (T)(std::numeric_limits<int8_t>::max)();
			constexpr auto MID_HIGT = MID_LOW + Math::ONE<T>;

			auto v = (T)value - MID_LOW;
			if (v < Math::ZERO<T>) {
				return DeviceStateValue(v) * Math::RECIPROCAL<DeviceStateValue(MID_LOW)>;
			} else if (v > Math::ZERO<T>) {
				return DeviceStateValue(v) * Math::RECIPROCAL<DeviceStateValue(MID_HIGT)>;
			}

			return Math::ZERO<DeviceStateValue>;
		}

		inline static DeviceStateValue AE_CALL _normalizeAxis(uint8_t value) {
			return DeviceStateValue(value) * Math::RECIPROCAL<DeviceStateValue((std::numeric_limits<uint8_t>::max)())>;
		}

		void static AE_CALL _translateTouch(uint8_t* data, DeviceTouchStateValue* states);

		inline static DeviceStateValue AE_CALL _translateButton(uint8_t value) {
			return value ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		}
	};
}