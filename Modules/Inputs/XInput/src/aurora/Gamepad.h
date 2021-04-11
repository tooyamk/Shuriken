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
		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		template<Arithmetic T> inline static constexpr T NUMBER_255 = T(255);
		template<Arithmetic T> inline static constexpr T NUMBER_32767 = T(32767);
		template<Arithmetic T> inline static constexpr T NUMBER_65535 = T(65535);

		DWORD _index;
		IntrusivePtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;

		mutable std::shared_mutex _mutex;
		XINPUT_STATE _state;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadKeyCode, DeviceState::ValueType> _deadZone;

		inline float32_t AE_CALL _getDeadZone(GamepadKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Math::ZERO<float32_t>;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setDeadZone(GamepadKeyCode keyCode, DeviceState::ValueType deadZone);

		void AE_CALL _setVibration(DeviceState::ValueType left, DeviceState::ValueType right);

		DeviceState::CountType AE_CALL _getStick(SHORT x, SHORT y, GamepadKeyCode key, DeviceState::ValueType* data, DeviceState::CountType count) const;
		DeviceState::CountType AE_CALL _getTrigger(SHORT t, GamepadKeyCode key, DeviceState::ValueType& data) const;

		void AE_CALL _dispatchStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(SHORT ori, SHORT cur, GamepadKeyCode key);
		void AE_CALL _dispatchButton(WORD ori, WORD cur, uint16_t flags, GamepadKeyCode key);

		inline static DeviceState::ValueType AE_CALL _translateDeadZone01(DeviceState::ValueType value, DeviceState::ValueType dz, bool inDz) {
			return inDz ? Math::ZERO<DeviceState::ValueType> : (value - dz) / (Math::ONE<DeviceState::ValueType> - dz);
		}

		template<bool FLIP>
		inline static DeviceState::ValueType AE_CALL _translateStick(SHORT value) {
			if constexpr (FLIP) {
				return DeviceState::ValueType(value) * Math::NEGATIVE<Math::RECIPROCAL<NUMBER_32767<DeviceState::ValueType>>>;
			} else {
				return DeviceState::ValueType(value) * Math::RECIPROCAL<NUMBER_32767<DeviceState::ValueType>>;
			}
		}
		inline static DeviceState::ValueType AE_CALL _translateTrigger(SHORT value) {
			return DeviceState::ValueType(value) * Math::RECIPROCAL<NUMBER_255<DeviceState::ValueType>>;
		}
		static DeviceState::ValueType AE_CALL _translateDpad(WORD value);
		inline static DeviceState::ValueType AE_CALL _translateButton(WORD value) {
			return value ? Math::ONE<DeviceState::ValueType> : Math::ZERO<DeviceState::ValueType>;
		}
	};
}