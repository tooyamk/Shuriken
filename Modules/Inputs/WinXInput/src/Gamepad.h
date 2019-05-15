#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::inputs::win_xinput {
	class Input;

	class AE_MODULE_DLL Gamepad : public IInputDevice {
	public:
		Gamepad(Input& input,  const DeviceInfo& info);
		virtual ~Gamepad();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(ui32 keyCode, f32 deadZone) override;
		virtual void AE_CALL setVibration(f32 left, f32 right) override;

	private:
		DWORD _index;
		RefPtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;

		XINPUT_STATE _state;
		XINPUT_VIBRATION _vibration;

		std::unordered_map<ui32, f32> _deadZone;

		inline f32 AE_CALL _getDeadZone(GamepadKeyCode key) const {
			if (auto itr = _deadZone.find((ui32)key); itr == _deadZone.end()) {
				return 0.f;
			} else {
				return itr->second;
			}
		}

		ui32 AE_CALL _getStick(SHORT x, SHORT y, GamepadKeyCode key, f32* data, ui32 count) const;
		ui32 AE_CALL _getTrigger(SHORT t, GamepadKeyCode key, f32& data) const;

		void AE_CALL _updateStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key);
		void AE_CALL _updateTrigger(SHORT ori, SHORT cur, GamepadKeyCode key);
		void AE_CALL _updateButton(WORD ori, WORD cur, ui16 flag, GamepadKeyCode key);

		inline static f32 AE_CALL _translateDeadZone0_1(f32 value, f32 dz, bool inDz) {
			return inDz ? 0.f : (value - dz) / (1.f - dz);
		}

		template<bool FLIP>
		inline static f32 AE_CALL _translateStick(SHORT value) {
			auto v = f32(value) / 32767.f;
			if constexpr (FLIP) v = -v;
			return v;
		}
		inline static f32 AE_CALL _translateTrigger(SHORT value) {
			return f32(value) / 255.f;
		}
		static f32 AE_CALL _translateDpad(WORD value);
		inline static f32 AE_CALL _translateButton(WORD value) {
			return value ? 1.f : 0.f;
		}
	};
}