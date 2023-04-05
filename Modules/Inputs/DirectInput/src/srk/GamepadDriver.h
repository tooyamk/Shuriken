#pragma once

#include "Base.h"
#include "srk/modules/inputs/GenericGamepad.h"

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		virtual ~GamepadDriver();

		static GamepadDriver* SRK_CALL create(Input& input, srk_IDirectInputDevice* dev);

		virtual size_t SRK_CALL getInputBufferLength() const override;
		virtual size_t SRK_CALL getOutputBufferLength() const override;

		virtual bool SRK_CALL init(void* inputBuffer, void* outputBuffer) override;

		virtual bool SRK_CALL isBufferReady(const void* buffer) const override;

		virtual std::optional<bool> SRK_CALL readFromDevice(void* inputBuffer) const override;
		virtual float32_t SRK_CALL readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const override;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputBuffer, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateEndCallback readStateEndCallback) const override;
		virtual void SRK_CALL customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* custom, DispatchCallback dispatchCallback) const override;

		virtual bool SRK_CALL writeToDevice(const void* outputBuffer) const override;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateEndCallback writeStateEndCallback) const override;

		virtual void SRK_CALL setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const override;

		virtual void SRK_CALL close() override;

	private:
		GamepadDriver(Input& input, srk_IDirectInputDevice* dev, const DIDEVCAPS& caps);

		static constexpr size_t HEADER_LENGTH = alignof(DIJOYSTATE);

		IntrusivePtr<Input> _input;
		srk_IDirectInputDevice* _dev;
		DIDEVCAPS _cpas;
		GamepadKeyCode _maxAxisKeyCode;
		GamepadKeyCode _minDpadKeyCode, _maxDpadKeyCode;
		GamepadKeyCode _maxButtonKeyCode;
	};
}