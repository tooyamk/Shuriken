#pragma once

#include "GamepadDriverBase.h"

#if SRK_OS != SRK_OS_WINDOWS
namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL GamepadDriver : public GamepadDriverBase {
	public:
		virtual ~GamepadDriver();

		static GamepadDriver* SRK_CALL create(Input& input, extensions::HIDDevice& hid, int32_t index);

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
		GamepadDriver(Input& input, extensions::HIDDevice& hid);

		struct InputCap {
			uint16_t offset;
			uint8_t size;
			uint32_t min;
			uint32_t max;
		};

		struct DeviceDesc {
			uint8_t inputReportID;
			uint32_t inputReportLength;
			std::vector<InputCap> inputAxes;
			std::vector<InputCap> inputDPads;
			std::vector<InputCap> inputButtons;
		};

		static void SRK_CALL _toString(extensions::HIDDevice& hid);
	};
}
#endif