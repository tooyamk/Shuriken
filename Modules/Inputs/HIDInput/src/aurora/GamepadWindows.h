#pragma once

#include "GamepadBase.h"

#if AE_OS == AE_OS_WIN
#include <hidsdi.h>

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public GamepadBase {
	public:
		Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		using InputBuffer = uint8_t[64];


		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_BUTTONS = 32;
		static constexpr extensions::HIDReportButtonPageType MAX_HID_BUTTON_PAGE = (extensions::HIDReportButtonPageType)((size_t)extensions::HIDReportButtonPageType::BUTTON_1 + MAX_BUTTONS - 1);
		static constexpr auto MAX_AXIS_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_1 + (MAX_AXES - 1));
		static constexpr auto MAX_BUTTON_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));


		struct AxisCap {
			bool valid;
			uint32_t min;
			float32_t lengthReciprocal;
		};


		struct DPadCap {
			bool valid;
			uint32_t min, max;
		};


		struct InputState {
			float32_t axes[MAX_AXES];
			uint32_t dpad;
			uint_t<MAX_BUTTONS> buttons;
		};


		PHIDP_PREPARSED_DATA _preparsedData;
		std::vector<HIDP_VALUE_CAPS> _inputValueCaps;
		size_t _maxValidAxes;
		AxisCap _axisCaps[MAX_AXES];
		DPadCap _dpadCap;

		mutable std::shared_mutex _inputMutex;
		InputState _inputState;

		void AE_CALL _setKeyMapping(const GamepadKeyMapping* mapping);

		void AE_CALL _readState(InputState& state, const uint8_t* inputBuffer, size_t inputBufferSize);

		inline static float32_t AE_CALL _readAxisVal(const InputState& state, GamepadKeyCode k, float32_t defaultVal) {
			using namespace aurora::enum_operators;

			if (k >= GamepadKeyCode::AXIS_1 && k <= MAX_AXIS_KEY) {
				return state.axes[(size_t)(k - GamepadKeyCode::AXIS_1)];
			} else {
				return defaultVal;
			}
		}

		inline static uint_t<MAX_BUTTONS> AE_CALL _readButtonVal(const InputState& state, GamepadKeyCode k) {
			using namespace aurora::enum_operators;

			if (k >= GamepadKeyCode::BUTTON_1 && k <= MAX_BUTTON_KEY) {
				return state.buttons & (Math::ONE<uint_t<MAX_BUTTONS>> << (size_t)(k - GamepadKeyCode::BUTTON_1));
			} else {
				return 0;
			}
		}

		DeviceState::CountType AE_CALL _getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const;
		DeviceState::CountType AE_CALL _getCombinedTrigger(GamepadKeyCode k, GamepadVirtualKeyCode vk, uint8_t index, DeviceStateValue& data) const;
		DeviceState::CountType AE_CALL _getAxis(GamepadKeyCode k, GamepadVirtualKeyCode vk, DeviceStateValue& data) const;

		void AE_CALL _dispatchStick(float32_t oldX, float32_t oldY, float32_t newX, float32_t newY, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchCombinedTrigger(float32_t oldVal, float32_t newVal);
		void AE_CALL _dispatchAxis(float32_t oldVal, float32_t newVal, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchDpad(uint32_t oldVal, uint32_t newVal);

		inline static DeviceStateValue AE_CALL _normalizeStick(float32_t value) {
			return value * 2.0f - 1.0f;
		}

		inline static void AE_CALL _normalizeCombinedAxis(float32_t value, DeviceStateValue& low, DeviceStateValue& high) {
			if (value < Math::ONE_HALF<decltype(value)>) {
				low = (Math::ONE_HALF<DeviceStateValue> - DeviceStateValue(value)) * Math::TWO<DeviceStateValue>;
				high = Math::ZERO<DeviceStateValue>;
			} else if (value > Math::ONE_HALF<decltype(value)>) {
				low = Math::ZERO<DeviceStateValue>;
				high = (DeviceStateValue(value) - Math::ONE_HALF<DeviceStateValue>) * Math::TWO<DeviceStateValue>;
			} else {
				low = Math::ZERO<DeviceStateValue>;
				high = Math::ZERO<DeviceStateValue>;
			}
		}

		static uint32_t AE_CALL _translateRangeVal(LONG rawVal, size_t numBits);

		static DeviceStateValue AE_CALL _translateDpad(uint32_t val, const DPadCap& cap);

		inline static DeviceStateValue AE_CALL _translateButton(uint_t<MAX_BUTTONS> value) {
			return value ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		}
	};
}
#endif