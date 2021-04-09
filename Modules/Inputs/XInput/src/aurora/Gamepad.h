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
		virtual Key::CountType AE_CALL getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;
		virtual void AE_CALL setDeadZone(Key::CodeType keyCode, Key::ValueType deadZone) override;
		virtual void AE_CALL setVibration(Key::ValueType left, Key::ValueType right) override;

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
		XINPUT_VIBRATION _vibration;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<Key::CodeType, Key::ValueType> _deadZone;

		inline Key::ValueType AE_CALL _getDeadZone(GamepadKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find((Key::CodeType)key); itr == _deadZone.end()) {
				return Math::ZERO<Key::ValueType>;
			} else {
				return itr->second;
			}
		}

		Key::CountType AE_CALL _getStick(SHORT x, SHORT y, GamepadKeyCode key, Key::ValueType* data, Key::CountType count) const;
		Key::CountType AE_CALL _getTrigger(SHORT t, GamepadKeyCode key, Key::ValueType& data) const;

		void AE_CALL _dispatchStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key);
		void AE_CALL _dispatchTrigger(SHORT ori, SHORT cur, GamepadKeyCode key);
		void AE_CALL _dispatchButton(WORD ori, WORD cur, uint16_t flags, GamepadKeyCode key);

		inline static Key::ValueType AE_CALL _translateDeadZone01(Key::ValueType value, Key::ValueType dz, bool inDz) {
			return inDz ? Math::ZERO<Key::ValueType> : (value - dz) / (Math::ONE<Key::ValueType> - dz);
		}

		template<bool FLIP>
		inline static Key::ValueType AE_CALL _translateStick(SHORT value) {
			if constexpr (FLIP) {
				return Key::ValueType(value) * Math::NEGATIVE<Math::RECIPROCAL<NUMBER_32767<Key::ValueType>>>;
			} else {
				return Key::ValueType(value) * Math::RECIPROCAL<NUMBER_32767<Key::ValueType>>;
			}
		}
		inline static Key::ValueType AE_CALL _translateTrigger(SHORT value) {
			return Key::ValueType(value) * Math::RECIPROCAL<NUMBER_255<Key::ValueType>>;
		}
		static Key::ValueType AE_CALL _translateDpad(WORD value);
		inline static Key::ValueType AE_CALL _translateButton(WORD value) {
			return value ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
		}
	};
}