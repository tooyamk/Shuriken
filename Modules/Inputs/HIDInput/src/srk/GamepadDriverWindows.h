#pragma once

#include "GamepadDriverBase.h"

#if SRK_OS == SRK_OS_WINDOWS
#include <hidsdi.h>

namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL GamepadDriver : public GamepadDriverBase {
	public:
		GamepadDriver(Input& input, extensions::HIDDevice& hid);
		virtual ~GamepadDriver();

		virtual size_t SRK_CALL getInputLength() const override;
		virtual size_t SRK_CALL getOutputLength() const override;

		virtual bool SRK_CALL init(void* inputState, void* outputState) override;

		virtual bool SRK_CALL isStateReady(const void* state) const override;

		virtual bool SRK_CALL readStateFromDevice(void* inputState) const override;
		virtual float32_t SRK_CALL readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const override;
		virtual float32_t SRK_CALL readDpadDataFromInputState(const void* inputState) const override;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const override;
		virtual void SRK_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const override;

		virtual bool SRK_CALL writeStateToDevice(const void* outputState) const override;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const override;

		virtual void SRK_CALL setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const override;

	private:
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
		DeviceStateValue _dpadUnit;

		static uint32_t SRK_CALL _translateRangeVal(LONG rawVal, size_t numBits);

		void SRK_CALL _parseInputState(InputState& state, const void* inputBuffer, size_t inputBufferSize) const;
	};
}
#endif