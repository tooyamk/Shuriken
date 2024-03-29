#pragma once

#include "Base.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericGamepad.h"

namespace srk::modules::inputs::evdev_input {
	class Input;

	class SRK_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		virtual ~GamepadDriver();

		static GamepadDriver* SRK_CALL create(Input& input, int32_t fd);

		virtual size_t SRK_CALL getInputBufferLength() const override;
		virtual size_t SRK_CALL getOutputBufferLength() const override;

		virtual bool SRK_CALL init(void* inputBuffer, void* outputBuffer) override;

		virtual bool SRK_CALL isBufferReady(const void* buffer) const override;

		virtual std::optional<bool> SRK_CALL readFromDevice(void* inputBuffer) const override;
		virtual float32_t SRK_CALL readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const override;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputBuffer, void* userData, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const override;
		virtual void SRK_CALL customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* userData, DispatchCallback dispatchCallback) const override;

		virtual bool SRK_CALL writeToDevice(const void* outputBuffer) const override;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* userData,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const override;

		virtual void SRK_CALL setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const override;

		virtual void SRK_CALL close() override;

	private:
		struct AxisDesc {
			uint32_t index;
			int32_t min;
			int32_t max;
			float32_t value;
		};


		struct ButtonDesc {
			uint16_t index;
			uint8_t value;
		};


		struct DeviceDesc {
			DeviceDesc() {}
			DeviceDesc(DeviceDesc&& other) : 
				fd(other.fd),
				inputAxes(std::move(other.inputAxes)),
				inputButtons(std::move(other.inputButtons)) {
			}

			int32_t fd;
			std::unordered_map<uint32_t, AxisDesc> inputAxes;
			std::unordered_map<uint32_t, ButtonDesc> inputButtons;
		};


		GamepadDriver(Input& input, DeviceDesc&& desc);

		static constexpr size_t HEADER_LENGTH = 4;

		IntrusivePtr<Input> _input;
		DeviceDesc _desc;

		size_t _inputBufferLength;
		size_t _axisBufferPos;
		size_t _buttonBufferPos;

		GamepadKeyCode _maxAxisKeyCode;
		GamepadKeyCode _maxButtonKeyCode;

		mutable AtomicLock _lock;
		mutable uint8_t* _inputBuffer;

		inline static float32_t SRK_CALL _foramtAxisValue(int32_t value, const AxisDesc& desc) {
			return (float32_t)(std::clamp(value, desc.min, desc.max) - desc.min) / (float32_t)(desc.max - desc.min);
		}

		inline float32_t& SRK_CALL _getAxisValue(void* state, size_t index) const {
			return ((float32_t*)((uint8_t*)state + _axisBufferPos))[index];
		}

		inline float32_t SRK_CALL _getAxisValue(const void* state, size_t index) const {
			return ((const float32_t*)((const uint8_t*)state + _axisBufferPos))[index];
		}

		inline void SRK_CALL _setButtonValue(void* state, uint32_t index, uint8_t val) const {
			auto& dst = ((uint8_t*)state + _buttonBufferPos)[index >> 3];
			auto shift = index & 0b111;
			dst = (dst & (~(1 << shift))) | (val << shift);
		}

		inline bool SRK_CALL _getButtonValue(const void* state, uint32_t index) const {
			return (((const uint8_t*)state + _buttonBufferPos)[index >> 3] & (1 << (index & 0b111))) != 0;
		}
	};
}