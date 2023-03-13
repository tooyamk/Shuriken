#include "InputModule.h"
#include "srk/events/IEventDispatcher.h"
#include <bitset>

namespace srk::modules::inputs {
	GamepadKeyMapper::GamepadKeyMapper() {
		clear();
	}

	GamepadKeyMapper::GamepadKeyMapper(const GamepadKeyMapper& other) {
		*this = other;
	}

	GamepadKeyMapper& GamepadKeyMapper::operator=(const GamepadKeyMapper& other) {
		for (size_t i = 0; i < _mapping.size(); ++i) _mapping[i] = other._mapping[i];
		return *this;
	}

	void GamepadKeyMapper::setDefault(uint8_t numAxes, uint8_t numDPads, uint8_t numButtons, bool axisUpIsBigValue) {
		using namespace srk::enum_operators;
		
		clear();

		std::vector<uint8_t> usedAxes(numAxes);
		memset(usedAxes.data(), 0, usedAxes.size());

		auto getAndSetAxis = [&]() {
			for (decltype(numAxes) i = 0; i < numAxes; ++i) {
				if (!usedAxes[i]) {
					usedAxes[i] = 1;
					return i;
				}
			}
			return Math::ZERO<decltype(numAxes)>;
		};

		auto setAxis = [&](auto& n, GamepadVirtualKeyCode vkBegin, size_t x, size_t y) {
			set(vkBegin, GamepadKeyCode::AXIS_1 + x, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			set(vkBegin + 1, GamepadKeyCode::AXIS_1 + x, GamepadKeyFlag::HALF_BIG);
			if (axisUpIsBigValue) {
				set(vkBegin + 2, GamepadKeyCode::AXIS_1 + y, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
				set(vkBegin + 3, GamepadKeyCode::AXIS_1 + y, GamepadKeyFlag::HALF_BIG);
			} else {
				set(vkBegin + 2, GamepadKeyCode::AXIS_1 + y, GamepadKeyFlag::HALF_BIG);
				set(vkBegin + 3, GamepadKeyCode::AXIS_1 + y, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			}

			usedAxes[x] = 1;
			usedAxes[y] = 1;
			n -= 2;
		};

		auto setBtn = [&](auto& n, GamepadVirtualKeyCode vk, GamepadKeyCode k) {
			if (n) {
				set(vk, k);
				--n;
			}
		};

		auto axes = numAxes;
		for (auto i = 0; i < 2; ++i) {
			if (axes >= 2) {
				auto sx = GamepadVirtualKeyCode::L_STICK_X_LEFT + (i << 2);

				if (!usedAxes[0] && !usedAxes[1]) {
					setAxis(axes, sx, 0, 1);
					continue;
				}
				if (!usedAxes[3] && !usedAxes[4]) {
					setAxis(axes, sx, 3, 4);
					continue;
				}
				if (!usedAxes[2] && !usedAxes[5]) {
					setAxis(axes, sx, 2, 5);
					continue;
				}

				{
					auto x = getAndSetAxis();
					auto y = getAndSetAxis();
					setAxis(axes, sx, x, y);
				}
			}
		}

		if (axes >= 1) {
			if (axes == 1) {
				auto i = getAndSetAxis();
				set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + i, GamepadKeyFlag::HALF_BIG);
				set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + i, GamepadKeyFlag::FLIP | GamepadKeyFlag::HALF_SMALL);
			} else {
				set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + getAndSetAxis());
				set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + getAndSetAxis());
			}
		}

		if (numDPads) {
			auto x = numAxes;
			auto y = numAxes + 1;
			set(GamepadVirtualKeyCode::DPAD_LEFT, GamepadKeyCode::AXIS_1 + x, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			set(GamepadVirtualKeyCode::DPAD_RIGHT, GamepadKeyCode::AXIS_1 + x, GamepadKeyFlag::HALF_BIG);
			set(GamepadVirtualKeyCode::DPAD_DOWN, GamepadKeyCode::AXIS_1 + y, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			set(GamepadVirtualKeyCode::DPAD_UP, GamepadKeyCode::AXIS_1 + y, GamepadKeyFlag::HALF_BIG);
		}

		auto btns = numButtons;
		setBtn(btns, GamepadVirtualKeyCode::A, GamepadKeyCode::BUTTON_1);
		setBtn(btns, GamepadVirtualKeyCode::B, GamepadKeyCode::BUTTON_1 + 1);
		setBtn(btns, GamepadVirtualKeyCode::X, GamepadKeyCode::BUTTON_1 + 2);
		setBtn(btns, GamepadVirtualKeyCode::Y, GamepadKeyCode::BUTTON_1 + 3);
		setBtn(btns, GamepadVirtualKeyCode::L_SHOULDER, GamepadKeyCode::BUTTON_1 + 4);
		setBtn(btns, GamepadVirtualKeyCode::R_SHOULDER, GamepadKeyCode::BUTTON_1 + 5);
		setBtn(btns, GamepadVirtualKeyCode::BACK, GamepadKeyCode::BUTTON_1 + 6);
		setBtn(btns, GamepadVirtualKeyCode::START, GamepadKeyCode::BUTTON_1 + 7);
		setBtn(btns, GamepadVirtualKeyCode::L_THUMB, GamepadKeyCode::BUTTON_1 + 8);
		setBtn(btns, GamepadVirtualKeyCode::R_THUMB, GamepadKeyCode::BUTTON_1 + 9);
	}

	bool GamepadKeyMapper::remove(GamepadVirtualKeyCode vk) {
		if (vk < VK_MIN || vk > VK_MAX) return false;

		_mapping[_getIndex(vk)].clear();
		return true;
	}

	void GamepadKeyMapper::removeUndefined() {
		using namespace srk::enum_operators;

		for (size_t i = 0; i < _mapping.size(); ++i) {
			if (auto vk = VK_MIN + i;
				(vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) ||
				(vk >= GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_BUTTON_END)) {
				_mapping[i].clear();
			}
		}
	}

	void GamepadKeyMapper::clear() {
		for (auto& cf : _mapping) cf.clear();
	}

	void GamepadKeyMapper::undefinedCompletion(size_t maxAxes, size_t maxButtons) {
		using namespace srk::enum_operators;

		if (maxAxes > 64) maxAxes = 64;
		if (maxButtons > 64) maxButtons = 64;

		auto maxAxisKey = Math::clamp(GamepadKeyCode::AXIS_1 + (maxAxes - 1), GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END);
		auto maxButtonKey = Math::clamp(GamepadKeyCode::BUTTON_1 + (maxButtons - 1), GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END);

		constexpr auto maxUndefinedAses = std::min<size_t>((size_t)(GamepadVirtualKeyCode::UNDEFINED_AXIS_END - GamepadVirtualKeyCode::UNDEFINED_AXIS_1 + 1), 64);
		constexpr auto maxUndefinedButtons = std::min<size_t>((size_t)(GamepadVirtualKeyCode::UNDEFINED_BUTTON_END - GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 + 1), 64);

		std::bitset<maxUndefinedAses> vkAxes;
		std::bitset<maxUndefinedButtons> vkButtons;
		std::bitset<64> kAxes, kButtons;

		for (size_t i = 0; i < _mapping.size(); ++i) {
			auto cf = _mapping[i].get();
			if (cf.code == GamepadKeyCode::UNDEFINED) continue;

			auto vk = VK_MIN + i;

			if (vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
				vkAxes[(size_t)(vk - GamepadVirtualKeyCode::UNDEFINED_AXIS_1)] = true;
			} else if (vk >= GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_BUTTON_END) {
				vkButtons[(size_t)(vk - GamepadVirtualKeyCode::UNDEFINED_BUTTON_1)] = true;
			}

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= maxAxisKey) {
				kAxes[(size_t)(cf.code - GamepadKeyCode::AXIS_1)] = true;
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= maxButtonKey) {
				kButtons[(size_t)(cf.code - GamepadKeyCode::BUTTON_1)] = true;
			}
		}

		for (size_t i = 0; i < maxAxes; ++i) {
			if (!kAxes[i]) {
				for (size_t j = 0; j < vkAxes.size(); ++j) {
					if (!vkAxes[j]) {
						vkAxes[j] = true;
						_mapping[_getIndex(GamepadVirtualKeyCode::UNDEFINED_AXIS_1 + j)].set(GamepadKeyCode::AXIS_1 + i);
						break;
					}
				}
			}
		}

		for (size_t i = 0; i < maxButtons; ++i) {
			if (!kButtons[i]) {
				for (size_t j = 0; j < vkButtons.size(); ++j) {
					if (!vkButtons[j]) {
						vkButtons[j] = true;
						_mapping[_getIndex(GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 + j)].set(GamepadKeyCode::BUTTON_1 + i);
						break;
					}
				}
			}
		}
	}


	GamepadKeyDeadZone::GamepadKeyDeadZone() {
		clear();
	}

	GamepadKeyDeadZone::GamepadKeyDeadZone(const GamepadKeyDeadZone& other) {
		*this = other;
	}

	GamepadKeyDeadZone& GamepadKeyDeadZone::operator=(const GamepadKeyDeadZone& other) {
		for (size_t i = 0; i < _values.size(); ++i) _values[i] = other._values[i];
		return *this;
	}

	bool GamepadKeyDeadZone::remove(GamepadVirtualKeyCode vk) {
		if (vk < VK_MIN || vk > VK_MAX) return false;

		_values[_getIndex(vk)].clear();
		return true;
	}

	void GamepadKeyDeadZone::clear() {
		for (auto& i : _values) i.clear();
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

		auto validLen = deadZone[0] + Math::ONE<DeviceStateValue> - deadZone[1];
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

		out[0] = std::atan2(-y, x) + Math::PI_2<DeviceStateValue>;
		if (out[0] < Math::ZERO<DeviceStateValue>) out[0] += Math::PI2<DeviceStateValue>;
		if (outCount > 1) out[1] = translate(len2 < Math::ONE<DeviceStateValue> ? std::sqrt(len2) : Math::ONE<DeviceStateValue>, deadZone);

		return outCount >= 2 ? 2 : 1;
	}
}