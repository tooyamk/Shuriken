#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::win_xinput {
	class Input;

	class AE_MODULE_DLL Gamepad : public IInputDevice {
	public:
		Gamepad(Input* input,  const InputDeviceInfo& info);
		virtual ~Gamepad();

		virtual events::IEventDispatcher<InputDeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const InputDeviceInfo& AE_CALL getInfo() const override;
		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(ui32 keyCode, f32 deadZone) override;

	private:
		RefPtr<Input> _input;
		events::EventDispatcher<InputDeviceEvent> _eventDispatcher;
		InputDeviceInfo _info;

		XINPUT_STATE _state;

		std::unordered_map<ui32, f32> _deadZone;

		inline f32 AE_CALL _getDeadZone(GamepadKeyCode key) const {
			if (auto itr = _deadZone.find((ui32)key); itr == _deadZone.end()) {
				return 0.f;
			} else {
				return itr->second;
			}
		}

		inline static f32 AE_CALL _translateDeadZone0_1(f32 value, f32 dz, bool inDz) {
			return inDz ? 0.f : (value - dz) / (1.f - dz);
		}

		void AE_CALL _updateStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key);


		static f32 AE_CALL _translateStick(SHORT value, bool flip);
	};
}