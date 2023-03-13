#pragma once

#include "GamepadDriverBase.h"

#if SRK_OS == SRK_OS_WINDOWS
#include <hidsdi.h>

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
		static constexpr size_t HEADER_LENGTH = 1;


		struct DeviceDesc {
			struct Axis {
				uint32_t min;
				float32_t lengthReciprocal;
				extensions::HIDReportUsagePageType usagePage;
				extensions::HIDReportGenericDesktopPageType usage;
			};

			struct {
				bool valid;
				uint32_t min, max;
				float32_t unitAngle;
			} dpad;

			uint16_t inputReportByteLength;
			uint16_t outputReportByteLength;

			std::vector<Axis> axes;
			std::vector<extensions::HIDReportButtonPageType> buttons;

			std::unordered_map<extensions::HIDReportButtonPageType, uint32_t> buttonMapper;

			DeviceDesc() {}
			DeviceDesc(const DeviceDesc&) = delete;
			DeviceDesc(DeviceDesc&& other) noexcept :
				dpad(other.dpad),
				inputReportByteLength(other.inputReportByteLength),
				outputReportByteLength(other.outputReportByteLength),
				axes(std::move(other.axes)),
				buttons(std::move(other.buttons)),
				buttonMapper(std::move(other.buttonMapper)) {
			}
		};


		PHIDP_PREPARSED_DATA _preparsedData;

		DeviceDesc _desc;

		struct {
			uint32_t axis;
			uint32_t dpad;
			uint32_t button;
		} _offset;

		GamepadKeyCode _maxAxisKeyCode;
		GamepadKeyCode _maxButtonKeyCode;

		GamepadDriver(Input& input, extensions::HIDDevice& hid, PHIDP_PREPARSED_DATA preparsedData, DeviceDesc&& desc);

		static uint32_t SRK_CALL _translateRangeVal(LONG rawVal, size_t numBits);
	};
}
#endif