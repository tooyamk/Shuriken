#pragma once

#include "Base.h"
#include "srk/modules/inputs/GenericGamepad.h"

namespace srk::modules::inputs::xinput {
	class Input;

	class SRK_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		GamepadDriver(Input& input, DWORD index);
		virtual ~GamepadDriver();

		virtual size_t SRK_CALL getInputBufferLength() const override;
		virtual size_t SRK_CALL getOutputBufferLength() const override;

		virtual bool SRK_CALL init(void* inputBuffer, void* outputBuffer) override;

		virtual bool SRK_CALL isBufferReady(const void* buffer) const override;

		virtual std::optional<bool> SRK_CALL readFromDevice(void* inputBuffer) const override;
		virtual float32_t SRK_CALL readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const override;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputBuffer, void* userData, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateEndCallback readStateEndCallback) const override;
		virtual void SRK_CALL customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* userData, DispatchCallback dispatchCallback) const override;

		virtual bool SRK_CALL writeToDevice(const void* outputBuffer) const override;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* userData,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateEndCallback writeStateEndCallback) const override;

		virtual void SRK_CALL setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const override;

		virtual void SRK_CALL close() override;

	private:
		static constexpr size_t HEADER_LENGTH = alignof(XINPUT_STATE);
		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_HATS = 1;
		static constexpr size_t MAX_BUTTONS = 10;
		static constexpr uint16_t BUTTON_MASK[] = {
			XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
			XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
			XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START,
			XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB
		};
		static constexpr auto MAX_AXIS_KEY_CODE = (GamepadKeyCode)(std::to_underlying(GamepadKeyCode::AXIS_1) + (MAX_AXES - 1));
		static constexpr auto MAX_BUTTON_KEY_CODE = (GamepadKeyCode)(std::to_underlying(GamepadKeyCode::BUTTON_1) + (MAX_BUTTONS - 1));


		IntrusivePtr<Input> _input;
		DWORD _index;

		void SRK_CALL _setVibration(DeviceStateValue left, DeviceStateValue right) const;

		inline static float32_t SRK_CALL _normalizeThumb(SHORT val) {
			constexpr auto N = (int32_t)(std::numeric_limits<int16_t>::max)() + 1;

			return float32_t((int32_t)val + N) / 65535.0f;
		}

		inline static float32_t SRK_CALL _normalizeTrigger(BYTE val) {
			return float32_t(val) / 255.0f;
		}
	};
}