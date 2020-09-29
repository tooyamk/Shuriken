#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::xinput {
	class Input;

	class AE_MODULE_DLL Gamepad : public IInputDevice {
	public:
		Gamepad(Input& input,  const DeviceInfo& info);
		virtual ~Gamepad();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual uint32_t AE_CALL getKeyState (uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone (uint32_t keyCode, float32_t deadZone) override;
		virtual void AE_CALL setVibration(float32_t left, float32_t right) override;

	private:
		DWORD _index;
		RefPtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;

		XINPUT_STATE _state;
		XINPUT_VIBRATION _vibration;

		std::unordered_map<uint32_t, float32_t> _deadZone;

		inline float32_t AE_CALL _getDeadZone(GamepadKeyCode key) const {
			if (auto itr = _deadZone.find((uint32_t)key); itr == _deadZone.end()) {
				return 0.f;
			} else {
				return itr->second;
			}
		}

		uint32_t AE_CALL _getStick(SHORT x, SHORT y, GamepadKeyCode key, float32_t* data, uint32_t count) const;
		uint32_t AE_CALL _getTrigger(SHORT t, GamepadKeyCode key, float32_t& data) const;

		void AE_CALL _updateStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key);
		void AE_CALL _updateTrigger(SHORT ori, SHORT cur, GamepadKeyCode key);
		void AE_CALL _updateButton(WORD ori, WORD cur, uint16_t flags, GamepadKeyCode key);

		inline static float32_t AE_CALL _translateDeadZone0_1(float32_t value, float32_t dz, bool inDz) {
			return inDz ? 0.f : (value - dz) / (1.f - dz);
		}

		template<bool FLIP>
		inline static float32_t AE_CALL _translateStick(SHORT value) {
			if constexpr (FLIP) {
				return float32_t(value) / -32767.f;
			} else {
				return float32_t(value) / 32767.f;
			}
		}
		inline static float32_t AE_CALL _translateTrigger(SHORT value) {
			return float32_t(value) / 255.f;
		}
		static float32_t AE_CALL _translateDpad(WORD value);
		inline static float32_t AE_CALL _translateButton(WORD value) {
			return value ? 1.f : 0.f;
		}
	};
}