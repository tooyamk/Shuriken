#include "GamepadDriver.h"
#include "Input.h"
#include "srk/Printer.h"
#include <linux/input.h>

namespace srk::modules::inputs::evdev_input {
	GamepadDriver::GamepadDriver(Input& input, DeviceDesc&& desc) :
		_input(input),
		_desc(std::move(desc)) {
		using namespace srk::enum_operators;

		_axisBufferPos = HEADER_LENGTH;
		_buttonBufferPos = _axisBufferPos + sizeof(int32_t) * _desc.inputAxes.size();

		_maxAxisKeyCode = GamepadKeyCode::AXIS_1 + _desc.inputAxes.size() - 1;
		_maxButtonKeyCode = GamepadKeyCode::BUTTON_1 + _desc.inputButtons.size() - 1;

		_inputBufferLength = HEADER_LENGTH + sizeof(float32_t) * _desc.inputAxes.size() + ((_desc.inputButtons.size() + 7) >> 3);
		_inputBuffer = new uint8_t[_inputBufferLength];
	}

	GamepadDriver::~GamepadDriver() {
		close();
	}

	GamepadDriver* GamepadDriver::create(Input& input, int32_t fd) {
		using namespace std::string_view_literals;

		DeviceDesc desc;
		desc.fd = fd;

		uint8_t bits[(std::max(ABS_CNT, KEY_CNT) + 7) >> 3];

		auto len = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(bits)), bits); 
		if (len < 0) {
			printaln(L"EVIOCGBIT EV_ABS error"sv);
			return nullptr;
		}
		size_t index = 0;
		Input::traverseBits(bits, len, 0, [](size_t code, DeviceDesc& desc, auto& index) {
			if ((code >= ABS_X && code <= ABS_RZ) || (code >= ABS_HAT0X && code <= ABS_HAT3Y)) {
				auto& axis = desc.inputAxes.emplace(std::piecewise_construct, std::forward_as_tuple(code), std::forward_as_tuple()).first->second;
				axis.index = index++;
			}

			return 0;
		}, desc, index);

		for (auto& itr : desc.inputAxes) {
			if (ioctl(fd, EVIOCGABS(itr.first), bits) < 0) {
				printaln(L"EVIOCGABS error"sv);
				return nullptr;
			}

			auto info = (input_absinfo*)bits;
			itr.second.min = info->minimum;
			itr.second.max = info->maximum;
			itr.second.value = _foramtAxisValue(info->value, itr.second);
		}

		len = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits); 
		if (len < 0) {
			printaln(L"EVIOCGBIT EV_KEY error"sv);
			return nullptr;
		}
		index = 0;
		Input::traverseBits(bits, len, 0, [](size_t code, DeviceDesc& desc, auto& index) {
			if (code >= BTN_GAMEPAD && code <= BTN_THUMBR) {
				auto& btn = desc.inputButtons.emplace(std::piecewise_construct, std::forward_as_tuple(code), std::forward_as_tuple()).first->second;
				btn.index = index++;
			}

			return 0;
		}, desc, index);

		len = ioctl(fd, EVIOCGKEY(sizeof(bits)), bits);
		if (len < 0) {
			printaln(L"EVIOCGKEY error"sv);
			return nullptr;
		}

		for (auto& itr : desc.inputButtons) {
			auto nBytes = itr.first >> 3;
			auto nBits = itr.first & 0b111;
			itr.second.value = (bits[nBytes] & (1 << (nBits))) == 0 ? 0 : 1;
		}		

		return new GamepadDriver(input, std::move(desc));
	}

	size_t GamepadDriver::getInputBufferLength() const {
		return _inputBufferLength;
	}

	size_t GamepadDriver::getOutputBufferLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputBuffer, void* outputBuffer) {
		((uint8_t*)_inputBuffer)[0] = 1;
		for (auto& itr : _desc.inputAxes) _getAxisValue(_inputBuffer, itr.second.index) = itr.second.value;
		for (auto& itr : _desc.inputButtons) _setButtonValue(_inputBuffer, itr.second.index, itr.second.value);

		memcpy(inputBuffer, _inputBuffer, _inputBufferLength);

		//ioctl(_desc.fd, EVIOCGRAB, 1);

		return true;
	}

	bool GamepadDriver::isBufferReady(const void* buffer) const {
		return ((const uint8_t*)buffer)[0];
	}

	std::optional<bool> GamepadDriver::readFromDevice(void* inputBuffer) const {
		using namespace std::string_view_literals;
		
		input_event evts[8];
		auto changed = false;
		do {
			auto len = read(_desc.fd, evts, sizeof(evts));
			if (len < 0) break;

			if (len && (len % sizeof(input_event)) != 0) break;

			for (size_t i = 0, n = len / sizeof(input_event); i < n; ++i) {
				auto& evt = evts[i];
				//if (evt.type == EV_SYN || evt.type == EV_MSC) continue;
				switch (evt.type) {
				case EV_KEY:
				{
					if (auto itr = _desc.inputButtons.find(evt.code); itr != _desc.inputButtons.end()) {
						changed = true;
						auto val = evt.value == 0 ? 0 : 1;

						std::scoped_lock lock(_lock);
						_setButtonValue(_inputBuffer, itr->second.index, val);
					} else {
						//printaln(L"key  code:"sv, String::toString(evt.code, 16), L"  value:"sv, evt.value);
					}

					break;
				}
				case EV_ABS:
				{
					if (auto itr = _desc.inputAxes.find(evt.code); itr != _desc.inputAxes.end()) {
						changed = true;
						auto val = _foramtAxisValue(evt.value, itr->second);

						std::scoped_lock lock(_lock);
						_getAxisValue(_inputBuffer, itr->second.index) = val;
					} else {
						//printaln(L"abs  code:"sv, String::toString(evt.code, 16), L"  value:"sv, evt.value);
					}

					break;
				}
				default:
					//printaln(L"type:"sv, evt.type, L"  code:"sv, evt.code, L"  value:"sv, evt.value);
					break;
				}
			}
		} while (true);

		if (!changed) return std::make_optional(false);

		{
			std::scoped_lock lock(_lock);
			memcpy(inputBuffer, _inputBuffer, _inputBufferLength);
		}

		return std::make_optional(true);
	}

	float32_t GamepadDriver::readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const {
		using namespace srk::enum_operators;

		if (!isBufferReady(inputBuffer)) return -1.0f;

		if (keyCode >= GamepadKeyCode::AXIS_1 && keyCode <= _maxAxisKeyCode) {
			return _getAxisValue(inputBuffer, (size_t)(keyCode - GamepadKeyCode::AXIS_1));
		} else if (keyCode >= GamepadKeyCode::BUTTON_1 && keyCode <= _maxButtonKeyCode) {
			return _getButtonValue(inputBuffer, (size_t)(keyCode - GamepadKeyCode::BUTTON_1)) ? 1.0f : 0.0f;
		} else {
			return -1.0f;
		}
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputBuffer, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* custom, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeToDevice(const void* outputBuffer) const {
		return true;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		using namespace srk::enum_operators;

		auto trySetAxis = [](GamepadKeyMapper& dst, const DeviceDesc& desc, GamepadVirtualKeyCode vk, uint32_t evk, GamepadKeyFlag flags) {
			if (auto itr = desc.inputAxes.find(evk); itr != desc.inputAxes.end()) dst.set(vk, GamepadKeyCode::AXIS_1 + itr->second.index, flags);
		};

		auto trySetBtn = [](GamepadKeyMapper& dst, const DeviceDesc& desc, GamepadVirtualKeyCode vk, uint32_t evk) {
			if (auto itr = desc.inputButtons.find(evk); itr != desc.inputButtons.end()) dst.set(vk, GamepadKeyCode::BUTTON_1 + itr->second.index);
		};

		if (src) {
			dst = *src;
		} else {
			dst.clear();

			trySetAxis(dst, _desc, GamepadVirtualKeyCode::L_STICK_X_LEFT, ABS_X, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::L_STICK_X_RIGHT, ABS_X, GamepadKeyFlag::HALF_BIG);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::L_STICK_Y_DOWN, ABS_Y, GamepadKeyFlag::HALF_BIG);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::L_STICK_Y_UP, ABS_Y, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			trySetAxis(dst, _desc, GamepadVirtualKeyCode::R_STICK_X_LEFT, ABS_RX, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::R_STICK_X_RIGHT, ABS_RX, GamepadKeyFlag::HALF_BIG);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::R_STICK_Y_DOWN, ABS_RY, GamepadKeyFlag::HALF_BIG);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::R_STICK_Y_UP, ABS_RY, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			trySetAxis(dst, _desc, GamepadVirtualKeyCode::L_TRIGGER, ABS_Z, GamepadKeyFlag::NONE);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::R_TRIGGER, ABS_RZ, GamepadKeyFlag::NONE);

			trySetAxis(dst, _desc, GamepadVirtualKeyCode::DPAD_LEFT, ABS_HAT0X, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::DPAD_RIGHT, ABS_HAT0X, GamepadKeyFlag::HALF_BIG);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::DPAD_DOWN, ABS_HAT0Y, GamepadKeyFlag::HALF_BIG);
			trySetAxis(dst, _desc, GamepadVirtualKeyCode::DPAD_UP, ABS_HAT0Y, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			trySetBtn(dst, _desc, GamepadVirtualKeyCode::A, BTN_A);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::B, BTN_B);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::X, BTN_X);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::Y, BTN_Y);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::L_SHOULDER, BTN_TL);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::R_SHOULDER, BTN_TR);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::L_TRIGGER_BUTTON, BTN_TL2);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::R_TRIGGER_BUTTON, BTN_TR2);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::SELECT, BTN_SELECT);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::START, BTN_START);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::L_THUMB, BTN_THUMBL);
			trySetBtn(dst, _desc, GamepadVirtualKeyCode::R_THUMB, BTN_THUMBR);
		}

		dst.undefinedCompletion<GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END, GamepadVirtualKeyCode::UNDEFINED_AXIS_1>(_desc.inputAxes.size());
		dst.undefinedCompletion<GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1>(_desc.inputButtons.size());
	}

	void GamepadDriver::close() {
		if (_desc.fd < 0) return;

		//ioctl(_desc.fd, EVIOCGRAB, 0);
		::close(_desc.fd);
		delete[] _inputBuffer;
		_desc.fd = -1;
	}
}