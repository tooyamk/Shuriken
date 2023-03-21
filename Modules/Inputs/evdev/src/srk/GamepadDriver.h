#pragma once

#include "DeviceBase.h"
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
		struct DeviceCap {
			DeviceCap() {}
			DeviceCap(DeviceCap&& other) : 
				fd(other.fd),
				axes(std::move(other.axes)),
				buttons(std::move(other.buttons)) {
			}

			int32_t fd;
			std::unordered_map<uint32_t, uint32_t> axes;
			std::unordered_map<uint32_t, uint32_t> buttons;
		};


		GamepadDriver(Input& input, DeviceCap&& cap);

		static constexpr auto MIN_AXIS_KEY_CODE = ABS_X;
		static constexpr auto MAX_AXIS_KEY_CODE = ABS_RZ;
		static constexpr auto MIN_BUTTON_KEY_CODE = BTN_GAMEPAD;
		static constexpr auto MAX_BUTTON_KEY_CODE = BTN_THUMBR;

		struct InputState {
			float32_t axis[MAX_AXIS_KEY_CODE - MIN_AXIS_KEY_CODE + 1];
			bool buttons[MAX_BUTTON_KEY_CODE - MIN_BUTTON_KEY_CODE + 1];
		};

		static constexpr size_t HEADER_LENGTH = alignof(InputState);

		IntrusivePtr<Input> _input;
		DeviceCap _cap;

		static void SRK_CALL _recordInput(std::unordered_map<uint32_t, uint32_t>& map, uint8_t* bits, size_t len);
	};
}