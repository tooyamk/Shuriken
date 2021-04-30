#pragma once

#include "DeviceBase.h"
#include "aurora/modules/inputs/GenericGamepad.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		GamepadDriver(Input& input, LPDIRECTINPUTDEVICE8 dev);
		virtual ~GamepadDriver();

		virtual size_t AE_CALL getInputLength() const override;
		virtual size_t AE_CALL getOutputLength() const override;

		virtual bool AE_CALL init(void* inputState, void* outputState) override;

		virtual bool AE_CALL readStateFromDevice(void* inputState) const override;
		virtual float32_t AE_CALL readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const override;
		virtual float32_t AE_CALL readDpadDataFromInputState(const void* inputState) const override;
		virtual DeviceState::CountType AE_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputState, void* custom, ReadStateStartCallback readStateStartCallback, ReadStateEndCallback readStateEndCallback) const override;
		virtual void AE_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const override;

		virtual bool AE_CALL writeStateToDevice(const void* outputState) const override;
		virtual DeviceState::CountType AE_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* custom, WriteToOutputStateCallback writeToOutputStateCallback) const override;

		virtual void AE_CALL setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const override;

	private:
		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_BUTTONS = 32;
		static constexpr auto MAX_AXIS_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_1 + (MAX_AXES - 1));
		static constexpr auto MAX_BUTTON_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));


		IntrusivePtr<Input> _input;
		LPDIRECTINPUTDEVICE8 _dev;
	};
}