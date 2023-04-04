#pragma once

#include "Base.h"
#include "srk/modules/inputs/GenericGamepad.h"

namespace srk::modules::inputs::xinput {
	class Input;

	class SRK_MODULE_DLL GamepadDriver : public IGenericGamepadDriver {
	public:
		GamepadDriver(Input& input, DWORD index);
		virtual ~GamepadDriver();

		virtual size_t SRK_CALL getInputLength() const override;
		virtual size_t SRK_CALL getOutputLength() const override;

		virtual bool SRK_CALL init(void* inputState, void* outputState) override;

		virtual bool SRK_CALL isStateReady(const void* state) const override;

		virtual std::optional<bool> SRK_CALL readStateFromDevice(void* inputState) const override;
		virtual float32_t SRK_CALL readDataFromInputState(const void* inputState, GamepadKeyCode keyCode) const override;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const override;
		virtual void SRK_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const override;

		virtual bool SRK_CALL writeStateToDevice(const void* outputState) const override;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const override;

		virtual void SRK_CALL setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const override;

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
		static constexpr auto MAX_AXIS_KEY_CODE = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_1 + (MAX_AXES - 1));
		static constexpr auto MAX_BUTTON_KEY_CODE = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));


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