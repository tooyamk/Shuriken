#pragma once

#include "DeviceBase.h"
#include "math/Math.h"

namespace aurora::modules::inputs::win_direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(ui32 keyCode, f32 deadZone) override;

	private:
		struct KeyMapping {
			ui8 LSTICK_X;
			ui8 LSTICK_Y;
			ui8 RSTICK_X;
			ui8 RSTICK_Y;
			ui8 LTRIGGER;
			ui8 RTRIGGER;

			std::unordered_map<ui8, GamepadKeyCode> BUTTONS;
		};


		DIJOYSTATE2 _state;
		const KeyMapping* _keyMapping;
		std::unordered_map<GamepadKeyCode, ui8> _enumToKeyMapping;

		std::unordered_map<ui32, f32> _deadZone;

		inline f32 AE_CALL _getDeadZone(GamepadKeyCode key) const {
			if (auto itr = _deadZone.find((ui32)key); itr == _deadZone.end()) {
				return 0.f;
			} else {
				return itr->second;
			}
		}

		bool AE_CALL _checkInvalidData(const DIJOYSTATE2& state);

		ui32 AE_CALL _getStick(LONG x, LONG y, GamepadKeyCode key, f32* data, ui32 count) const;
		ui32 AE_CALL _getTrigger(LONG t, GamepadKeyCode key, ui8 index, f32& data) const;
		ui32 AE_CALL _getTriggerSeparate(LONG t, GamepadKeyCode key, f32& data) const;

		void AE_CALL _updateStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key);
		void AE_CALL _updateTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey);
		void AE_CALL _updateTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key);

		inline static f32 AE_CALL _translateDeadZone0_1(f32 value, f32 dz, bool inDz) {
			return inDz ? 0.f : (value - dz) / (1.f - dz);
		}

		static f32 AE_CALL _translateStick(LONG value);
		static void AE_CALL _translateTrigger(LONG value, f32& l, f32& r);
		inline static f32 AE_CALL _translateTriggerSeparate(LONG value) {
			return f32(value) / 65535.f;
		}
		inline static f32 AE_CALL _translateDpad(DWORD value) {
			return (value == (std::numeric_limits<ui32>::max)()) ? -1.f : Math::rad(f32(value) * .01f);
		}
		inline static f32 AE_CALL _translateButton(DWORD value) {
			return value & 0x80 ? 1.f : 0.f;
		}

		static bool AE_CALL _isXInputDevice(const ::GUID& guid);

		static const KeyMapping DIRECT;
		static const KeyMapping XINPUT;
		static const KeyMapping DS4;
	};
}