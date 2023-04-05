#pragma once

#include "GamepadDriverBase.h"

namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL GamepadDriver : public GamepadDriverBase {
	public:
		virtual ~GamepadDriver();

		static GamepadDriver* SRK_CALL create(Input& input, extensions::HIDDevice& hid, int32_t index);

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

	private:
		static constexpr size_t HEADER_LENGTH = 1;


		struct InputDesc {
			uint8_t size;
			uint16_t offset;
			uint16_t usage;
			int32_t min;
			int32_t max;

			void SRK_CALL setMinMax(int32_t min, int32_t max, uint32_t maxSize) {
				if (max < min) {
					switch (maxSize) {
					case 1:
						max = (uint8_t)(int8_t)max;
						break;
					case 2:
						max = (uint16_t)(int16_t)max;
						break;
					default:
						break;
					}
				}

				this->min = min;
				this->max = max;
			}
		};


		struct DeviceDesc {
			uint8_t inputReportID;
			uint32_t inputReportLength;
			std::vector<InputDesc> inputAxes;
			std::vector<InputDesc> inputDPads;
			std::vector<InputDesc> inputButtons;

			DeviceDesc() {}
			DeviceDesc(const DeviceDesc&) = delete;
			DeviceDesc(DeviceDesc&& other) noexcept :
				inputReportID(other.inputReportID),
				inputReportLength(other.inputReportLength),
				inputAxes(std::move(other.inputAxes)),
				inputDPads(std::move(other.inputDPads)),
				inputButtons(std::move(other.inputButtons)) {
			}
		};


		GamepadDriver(Input& input, extensions::HIDDevice& hid, DeviceDesc&& desc);
		
		DeviceDesc _desc;

		GamepadKeyCode _maxAxisKeyCode;
		GamepadKeyCode _maxHatKeyCode;
		GamepadKeyCode _maxButtonKeyCode;

		static int32_t SRK_CALL _read(const InputDesc& desc, const uint8_t* data);
	};
}