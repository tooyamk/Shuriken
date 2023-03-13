#pragma once

#include "DeviceBase.h"
#include "srk/modules/inputs/GenericGamepad.h"

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		virtual ~GamepadDriver();

		static GamepadDriver* SRK_CALL create(Input& input, srk_IDirectInputDevice* dev);

		virtual size_t SRK_CALL getInputLength() const override;
		virtual size_t SRK_CALL getOutputLength() const override;

		virtual bool SRK_CALL init(void* inputState, void* outputState) override;

		virtual bool SRK_CALL isStateReady(const void* state) const override;

		virtual bool SRK_CALL readStateFromDevice(void* inputState) const override;
		virtual float32_t SRK_CALL readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const override;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const override;
		virtual void SRK_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const override;

		virtual bool SRK_CALL writeStateToDevice(const void* outputState) const override;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const override;

		virtual void SRK_CALL setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const override;

	private:
		GamepadDriver(Input& input, srk_IDirectInputDevice* dev, const DIDEVCAPS& caps);

		static constexpr size_t HEADER_LENGTH = 1;

		IntrusivePtr<Input> _input;
		srk_IDirectInputDevice* _dev;
		DIDEVCAPS _cpas;
		GamepadKeyCode _maxAxisKeyCode;
		GamepadKeyCode _minDpadKeyCode, _maxDpadKeyCode;
		GamepadKeyCode _maxButtonKeyCode;
	};
}