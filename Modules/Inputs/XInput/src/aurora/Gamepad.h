#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::xinput {
	class Input;

	class AE_MODULE_DLL Gamepad : public IInputDevice {
	public:
		Gamepad(Input& input,  const DeviceInfo& info);
		virtual ~Gamepad();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_BUTTONS = 10;
		static constexpr uint16_t BUTTON_MASK[] = {
			XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
			XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
			XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START,
			XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB
		};
		static constexpr auto MAX_BUTTON_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));

		DWORD _index;
		IntrusivePtr<Input> _input;
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;

		mutable std::shared_mutex _mutex;
		XINPUT_STATE _state;
		GamepadKeyMapping _keyMapping;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadVirtualKeyCode, Vec2<DeviceStateValue>> _deadZone;

		bool AE_CALL _readState(XINPUT_STATE& state);

		inline Vec2<DeviceStateValue> AE_CALL _getDeadZone(GamepadVirtualKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Vec2<DeviceStateValue>::ZERO;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setKeyMapping(const GamepadKeyMapping* mapping);
		void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone);
		void AE_CALL _setVibration(DeviceStateValue left, DeviceStateValue right);

		static uint16_t AE_CALL _readAxisVal(const XINPUT_GAMEPAD& gamepad, GamepadKeyCode k, uint16_t defaultVal);

		inline static WORD AE_CALL _readButtonVal(WORD buttons, GamepadKeyCode k) {
			using namespace aurora::enum_operators;

			return k >= GamepadKeyCode::BUTTON_1 && k <= MAX_BUTTON_KEY ? buttons & BUTTON_MASK[(size_t)(k - GamepadKeyCode::BUTTON_1)] : 0;
		}

		DeviceState::CountType AE_CALL _getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const;

		void AE_CALL _dispatchStick(uint16_t oldX, uint16_t oldY, uint16_t newX, uint16_t newY, GamepadVirtualKeyCode key);
		void AE_CALL _dispatchAxis(uint16_t oldVal, uint16_t newVal, GamepadVirtualKeyCode key);

		inline static uint16_t AE_CALL _traslateRawThumbX(int32_t val) {
			constexpr auto N = (int32_t)(std::numeric_limits<int16_t>::max)() + 1;

			return (int32_t)val + N;
		}

		inline static uint16_t AE_CALL _traslateRawThumbY(int32_t val) {
			constexpr auto N = (int32_t)(std::numeric_limits<int16_t>::max)();

			return N - val;
		}

		inline static uint16_t AE_CALL _traslateRawTrigger(int32_t val) {
			return val * 257;
		}

		inline static DeviceStateValue AE_CALL _normalizeStick(uint16_t value) {
			using T = int32_t;

			constexpr auto MID_LOW = (T)(std::numeric_limits<int16_t>::max)();
			constexpr auto MID_HIGT = MID_LOW + Math::ONE<T>;

			auto v = (T)value - MID_LOW;
			if (v < Math::ZERO<T>) {
				return DeviceStateValue(v) * Math::RECIPROCAL<DeviceStateValue(MID_LOW)>;
			} else if (v > Math::ZERO<T>) {
				return DeviceStateValue(v) * Math::RECIPROCAL<DeviceStateValue(MID_HIGT)>;
			}

			return Math::ZERO<DeviceStateValue>;
		}

		inline static DeviceStateValue AE_CALL _normalizeAxis(uint16_t value) {
			return DeviceStateValue(value) * Math::RECIPROCAL<DeviceStateValue((std::numeric_limits<uint16_t>::max)())>;
		}

		static DeviceStateValue AE_CALL _translateDpad(WORD value);

		inline static DeviceStateValue AE_CALL _translateButton(WORD value) {
			return value ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		}
	};
}