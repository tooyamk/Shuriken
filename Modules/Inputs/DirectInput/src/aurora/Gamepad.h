#pragma once

#include "DeviceBase.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState (uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone (uint32_t keyCode, float32_t deadZone) override;

	private:
		struct KeyMapping {
			uint8_t LSTICK_X;
			uint8_t LSTICK_Y;
			uint8_t RSTICK_X;
			uint8_t RSTICK_Y;
			uint8_t LTRIGGER;
			uint8_t RTRIGGER;

			std::unordered_map<uint8_t, GamepadKeyCode> BUTTONS;
		};


		DIJOYSTATE2 _state;
		const KeyMapping* _keyMapping;
		std::unordered_map<GamepadKeyCode, uint8_t> _enumToKeyMapping;

		std::unordered_map<uint32_t, float32_t> _deadZone;

		inline float32_t AE_CALL _getDeadZone(GamepadKeyCode key) const {
			if (auto itr = _deadZone.find((uint32_t)key); itr == _deadZone.end()) {
				return 0.f;
			} else {
				return itr->second;
			}
		}

		bool AE_CALL _checkInvalidData(const DIJOYSTATE2& state);

		uint32_t AE_CALL _getStick(LONG x, LONG y, GamepadKeyCode key, float32_t* data, uint32_t count) const;
		uint32_t AE_CALL _getTrigger(LONG t, GamepadKeyCode key, uint8_t index, float32_t& data) const;
		uint32_t AE_CALL _getTriggerSeparate(LONG t, GamepadKeyCode key, float32_t& data) const;

		void AE_CALL _updateStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key);
		void AE_CALL _updateTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey);
		void AE_CALL _updateTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key);

		inline static float32_t AE_CALL _translateDeadZone0_1(float32_t value, float32_t dz, bool inDz) {
			return inDz ? 0.f : (value - dz) / (1.f - dz);
		}

		static float32_t AE_CALL _translateStick(LONG value);
		static void AE_CALL _translateTrigger(LONG value, float32_t& l, float32_t& r);
		inline static float32_t AE_CALL _translateTriggerSeparate(LONG value) {
			return float32_t(value) / 65535.f;
		}
		inline static float32_t AE_CALL _translateDpad(DWORD value) {
			return (value == (std::numeric_limits<uint32_t>::max)()) ? -1.f : Math::rad(float32_t(value) * .01f);
		}
		inline static float32_t AE_CALL _translateButton(DWORD value) {
			return value & 0x80 ? 1.f : 0.f;
		}

		static bool AE_CALL _isXInputDevice(const ::GUID& guid);

		static const KeyMapping DIRECT;
		static const KeyMapping XINPUT;
		static const KeyMapping DS4;
	};
}