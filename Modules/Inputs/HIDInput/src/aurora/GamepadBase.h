#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	template<size_t StateBufferSize, size_t ReadBufferSize>
	class GamepadBase : public DeviceBase<StateBufferSize, ReadBufferSize> {
	public:
		GamepadBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : DeviceBase<StateBufferSize, ReadBufferSize>(input, info, hid) {
			setDeadZone((Key::CodeType)GamepadKeyCode::L_STICK, Math::TWENTIETH<Key::ValueType>);
			setDeadZone((Key::CodeType)GamepadKeyCode::R_STICK, Math::TWENTIETH<Key::ValueType>);
			setDeadZone((Key::CodeType)GamepadKeyCode::L_TRIGGER, Math::TWENTIETH<Key::ValueType>);
			setDeadZone((Key::CodeType)GamepadKeyCode::R_TRIGGER, Math::TWENTIETH<Key::ValueType>);
		}

		virtual void AE_CALL setDeadZone(Key::CodeType keyCode, Key::ValueType deadZone) override {
			if (deadZone < Math::ZERO<Key::ValueType>) deadZone = -deadZone;

			std::scoped_lock lock(_deadZoneMutex);

			_deadZone.insert_or_assign(keyCode, deadZone);
		}

	protected:
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

		inline static Key::ValueType AE_CALL _translateDeadZone01(Key::ValueType value, Key::ValueType dz, bool inDz) {
			return inDz ? Math::ZERO<Key::ValueType> : (value - dz) / (Math::ONE<Key::ValueType> -dz);
		}
	};
}