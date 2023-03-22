#pragma once

#include "DeviceBase.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericGamepad.h"
#include <linux/input-event-codes.h>

namespace srk::modules::inputs::evdev {
	class SRK_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		virtual ~GamepadDriver();

		static GamepadDriver* SRK_CALL create(Input& input, int32_t fd);

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

		size_t _inputLength;
		size_t _axisBufferPos;
		size_t _buttonBufferPos;

		GamepadKeyCode _maxAxisKeyCode;
		GamepadKeyCode _maxHatKeyCode;
		GamepadKeyCode _maxButtonKeyCode;

		mutable AtomicLock<true, false> _lock;
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

		template<typename Fn>
		static void SRK_CALL _recordInput(uint8_t* bits, size_t len, Fn&& fn) {
			uint32_t index = 0; 
			uint32_t code = 0;
			for (decltype(len) i = 0; i < len; ++i) {
				auto val = bits[i];

				uint32_t n = 0;
				while (val) {
					if (val & 0b1) fn(code + n, index++);

					val >>= 1;
					++n;
				}

				code += 8;
			}
		}
	};
}