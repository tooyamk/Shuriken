#include "IInputModule.h"
#include "aurora/events/IEventDispatcher.h"
#include <bitset>

namespace aurora::modules::inputs {
	GamepadKeyMapping::GamepadKeyMapping() {
		clear();
	}

	GamepadKeyMapping::GamepadKeyMapping(const GamepadKeyMapping& other) {
		memcpy(_mapping, other._mapping, sizeof(_mapping));
	}

	bool GamepadKeyMapping::set(GamepadVirtualKeyCode vk, GamepadKeyCode k) {
		if (vk >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && vk <= GamepadVirtualKeyCode::SEPARATE_AXIS_END) {
			if (k != GamepadKeyCode::IGNORED && (k < GamepadKeyCode::AXIS_START || k > GamepadKeyCode::AXIS_END)) return false;
		} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
			if (k != GamepadKeyCode::IGNORED && (k < GamepadKeyCode::BUTTON_START || k > GamepadKeyCode::BUTTON_END)) return false;
		}

		_mapping[_getIndex(vk)] = k;

		return true;
	}

	bool GamepadKeyMapping::remove(GamepadVirtualKeyCode vk) {
		if (vk >= VK_MIN && vk <= VK_MAX) {
			_mapping[_getIndex(vk)] = GamepadKeyCode::UNDEFINED;

			return true;
		}

		return false;
	}

	void GamepadKeyMapping::undefinedCompletion(size_t maxAxes, size_t maxButtons) {
		using namespace aurora::enum_operators;

		if (maxAxes > 64) maxAxes = 64;
		if (maxButtons > 64) maxButtons = 64;

		auto maxAxisKey = Math::clamp(GamepadKeyCode::AXIS_1 + (maxAxes - 1), GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END);
		auto maxButtonKey = Math::clamp(GamepadKeyCode::BUTTON_1 + (maxButtons - 1), GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END);

		constexpr auto maxUndefinedAses = std::min<size_t>((size_t)(GamepadVirtualKeyCode::UNDEFINED_AXIS_END - GamepadVirtualKeyCode::UNDEFINED_AXIS_1 + 1), 64);
		constexpr auto maxUndefinedButtons = std::min<size_t>((size_t)(GamepadVirtualKeyCode::UNDEFINED_BUTTON_END - GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 + 1), 64);

		std::bitset<maxUndefinedAses> vkAxes;
		std::bitset<maxUndefinedButtons> vkButtons;
		std::bitset<64> kAxes, kButtons;

		for (size_t i = 0; i < COUNT; ++i) {
			auto k = _mapping[i];
			if (k == GamepadKeyCode::UNDEFINED) continue;

			auto vk = VK_MIN + i;

			if (vk >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && vk <= GamepadVirtualKeyCode::SEPARATE_AXIS_END) {
				if (vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1) vkAxes[(size_t)(vk - GamepadVirtualKeyCode::UNDEFINED_AXIS_1)] = true;
				if (k != GamepadKeyCode::IGNORED && k <= maxAxisKey) kAxes[(size_t)(k - GamepadKeyCode::AXIS_1)] = true;
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				if (vk >= GamepadVirtualKeyCode::UNDEFINED_BUTTON_1) vkButtons[(size_t)(vk - GamepadVirtualKeyCode::UNDEFINED_BUTTON_1)] = true;
				if (k != GamepadKeyCode::IGNORED && k <= maxButtonKey) kButtons[(size_t)(k - GamepadKeyCode::BUTTON_1)] = true;
			}
		}

		for (size_t i = 0; i < maxAxes; ++i) {
			if (!kAxes[i]) {
				for (size_t j = 0; j < vkAxes.size(); ++j) {
					if (!vkAxes[j]) {
						vkAxes[j] = true;
						_mapping[_getIndex(GamepadVirtualKeyCode::UNDEFINED_AXIS_1 + j)] = GamepadKeyCode::AXIS_1 + i;
						break;
					}
				}
			}
		}

		for (size_t i = 0; i < maxButtonKey; ++i) {
			if (!kButtons[i]) {
				for (size_t j = 0; j < vkButtons.size(); ++j) {
					if (!vkButtons[j]) {
						vkButtons[j] = true;
						_mapping[_getIndex(GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 + j)] = GamepadKeyCode::BUTTON_1 + i;
						break;
					}
				}
			}
		}
	}


	IInputDevice::~IInputDevice() {
	}


	IInputModule::IInputModule() {
	}

	IInputModule::~IInputModule() {
	}


	void IInputDevice::getStates(DeviceStateType type, DeviceState* states, size_t count) const {
		for (size_t i = 0; i < count; ++i) {
			auto& state = states[i];
			state.count = getState(type, state.code, state.values, state.count);
		}
	}

	void IInputDevice::setStates(DeviceStateType type, DeviceState* states, size_t count) {
		for (size_t i = 0; i < count; ++i) {
			auto& state = states[i];
			state.count = setState(type, state.code, state.values, state.count);
		}
	}

	DeviceStateValue IInputDevice::translate(DeviceStateValue value, const Vec2<DeviceStateValue>& deadZone) {
		auto validLen = deadZone[0] + Math::ONE<DeviceStateValue> -deadZone[1];
		if (validLen == Math::ZERO<DeviceStateValue>) return Math::ZERO<DeviceStateValue>;

		if (value >= deadZone[1]) {
			value -= deadZone[1] - deadZone[0];
		} else if (value > deadZone[0]) {
			value = deadZone[0];
		}

		return value / validLen;
	}

	DeviceState::CountType IInputDevice::translate(DeviceStateValue x, DeviceStateValue y, const Vec2<DeviceStateValue>& deadZone, DeviceStateValue* out, DeviceState::CountType outCount) {
		if (!outCount) return 0;

		if (x == Math::ZERO<DeviceStateValue> && y == Math::ZERO<DeviceStateValue>) {
			out[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
			if (outCount > 1) out[1] = Math::ZERO<DeviceStateValue>;

			return 1;
		}

		auto validLen = deadZone[0] + Math::ONE<DeviceStateValue> -deadZone[1];
		if (validLen == Math::ZERO<DeviceStateValue>) {
			out[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
			if (outCount > 1) out[1] = Math::ZERO<DeviceStateValue>;

			return 1;
		}

		auto dzMax2 = deadZone[1] * deadZone[1];
		auto len2 = x * x + y * y;

		if (deadZone[0] == Math::ZERO<DeviceStateValue> && len2 <= dzMax2) {
			out[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
			if (outCount > 1) out[1] = Math::ZERO<DeviceStateValue>;

			return 1;
		}

		out[0] = std::atan2(y, x) + Math::PI_2<DeviceStateValue>;
		if (out[0] < Math::ZERO<DeviceStateValue>) out[0] += Math::PI2<DeviceStateValue>;
		if (outCount > 1) out[1] = translate(len2 < Math::ONE<DeviceStateValue> ? std::sqrt(len2) : Math::ONE<DeviceStateValue>, deadZone);

		return outCount >= 2 ? 2 : 1;
	}
}