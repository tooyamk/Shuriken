#pragma once

#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(ui32 keyCode, f32 deadZone) override;

	private:
		DIJOYSTATE2 _state;
		const std::unordered_map<ui8, GamepadKeyCode>* _specifiedKeyToEnumMapping;
		std::unordered_map<GamepadKeyCode, ui8> _specifiedEnumToKeyMapping;

		std::unordered_map<ui32, f32> _deadZone;

		inline f32 _getDeadZone(GamepadKeyCode key) const {
			if (auto itr = _deadZone.find((ui32)key); itr == _deadZone.end()) {
				return 0.f;
			} else {
				return itr->second;
			}
		}

		ui32 _getStick(LONG x, LONG y, GamepadKeyCode key, f32* data, ui32 count) const;
		ui32 _getTrigger(LONG t, GamepadKeyCode key, f32& data) const;

		void _updateStick(LONG& oriX, LONG& oriY, LONG curX, LONG curY, GamepadKeyCode key);
		void _updateTrigger(LONG& ori, LONG cur, GamepadKeyCode key);

		inline static void _translateDeadZone0_1(f32& value, f32 dz, bool inDz) {
			value = inDz ? 0.f : (value - dz) / (1.f - dz);
		}
		inline static void _translateDeadZone_1_1(f32& value, f32 dz, bool inDz) {
			value = inDz ? 0.f : (value < 0.f ? dz + value : value - dz) / (1.f - dz);
		}

		static f32 _translateStick(LONG value);
		static f32 _translateTrigger(LONG value);
		static f32 _translateAngle(DWORD value);
		static f32 _translateButton(DWORD value);

		inline static const std::unordered_map<ui8, GamepadKeyCode> DS4{ { 13, GamepadKeyCode::TOUCH_PAD } };
	};
}