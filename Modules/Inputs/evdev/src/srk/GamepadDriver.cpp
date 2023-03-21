#include "GamepadDriver.h"
#include "Input.h"
#include "srk/Printer.h"
#include <linux/input.h>

namespace srk::modules::inputs::evdev {
	GamepadDriver::GamepadDriver(Input& input, DeviceCap&& cap) :
		_input(input),
		_cap(std::move(cap)) {
	}

	GamepadDriver::~GamepadDriver() {
		//ioctl(_fd, EVIOCGRAB, 0);
		::close(_cap.fd);
	}

	GamepadDriver* GamepadDriver::create(Input& input, int32_t fd) {
		using namespace std::literals;

		DeviceCap cap;
		cap.fd = fd;

		uint8_t bits[(std::max(ABS_CNT, KEY_CNT) + 7) >> 3];

		auto len = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(bits)), bits); 
		if (len < 0) {
			printaln(L"EVIOCGBIT EV_ABS error"sv);
			return nullptr;
		}
		_recordInput(cap.axes, bits, len);

		len = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits); 
		if (len < 0) {
			printaln(L"EVIOCGBIT EV_KEY error"sv);
			return nullptr;
		}
		_recordInput(cap.buttons, bits, len);

		return new GamepadDriver(input, std::move(cap));
	}

	void GamepadDriver::_recordInput(std::unordered_map<uint32_t, uint32_t>& map, uint8_t* bits, size_t len) {
		uint32_t index = 0; 
		uint32_t code = 0;
		for (decltype(len) i = 0; i < len; ++i) {
			auto val = bits[i];

			uint32_t n = 0;
			while (val) {
				if (val & 0b1) map.emplace(code + n, index++);

				val >>= 1;
				++n;
			}

			code += 8;
		}
	}

	size_t GamepadDriver::getInputLength() const {
		return HEADER_LENGTH + sizeof(InputState);
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		((uint8_t*)inputState)[0] = 0;

		return true;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return ((const uint8_t*)state)[0];
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		using namespace std::literals;

		auto& state = *(InputState*)((uint8_t*)inputState + HEADER_LENGTH);

		input_event evts[8];
		do {
			auto len = read(_cap.fd, evts, sizeof(evts));
			if (len < 0) break;

			if (len && (len % sizeof(input_event)) != 0) break;

			for (size_t i = 0, n = len / sizeof(input_event); i < n; ++i) {
				auto& evt = evts[i];
				if (evt.type == 0 || evt.type == 3) continue;
				switch (evt.type) {
				case EV_SYN:
				case EV_MSC:
					break;
				case EV_KEY:
				{
					if (evt.code >= MIN_BUTTON_KEY_CODE && evt.code <= MAX_BUTTON_KEY_CODE) {
						state.buttons[evt.code - MIN_BUTTON_KEY_CODE] = evt.value != 0;
					} else {
						printaln(L"key  code:"sv, String::toString(evt.code, 16), L"  value:"sv, evt.value);
					}

					break;
				}
				case EV_ABS:
					break;
				default:
					printaln(L"type:"sv, evt.type, L"  code:"sv, evt.code, L"  value:"sv, evt.value);
					break;
				}
			}
		} while (true);

		return false;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		return translate(defaultVal, cf.flags);
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeStateToDevice(const void* outputState) const {
		return false;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		using namespace srk::enum_operators;

		if (src) {
			dst = *src;
		} else {
			//dst.setDefault(_cpas.dwAxes, _cpas.dwPOVs, _cpas.dwButtons, false);
		}

		/*dst.undefinedCompletion<GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END, GamepadVirtualKeyCode::UNDEFINED_AXIS_1>(_cpas.dwAxes);
		dst.undefinedCompletion<GamepadKeyCode::HAT_1, GamepadKeyCode::HAT_END, GamepadVirtualKeyCode::UNDEFINED_HAT_1>(_cpas.dwPOVs);
		dst.undefinedCompletion<GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1>(_cpas.dwButtons);*/
	}
}