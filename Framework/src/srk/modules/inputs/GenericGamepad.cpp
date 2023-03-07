#include "GenericGamepad.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs {
	float32_t IGenericGamepadDriver::translate(float32_t state, GamepadKeyFlag flags) {
		using namespace srk::enum_operators;

		std::underlying_type_t<GamepadKeyFlag> i = 0;
		while (flags != GamepadKeyFlag::NONE) {
			auto flag = (GamepadKeyFlag)(1 << (i++));
			if ((flags & flag) != GamepadKeyFlag::NONE) {
				flags &= ~flag;

				switch (flag) {
				case GamepadKeyFlag::HALF_SMALL:
					state = state < Math::ONE_HALF<float32_t> ? state * Math::TWO<float32_t> : Math::ONE<float32_t>;
					break;
				case GamepadKeyFlag::HALF_BIG:
					state = state > Math::ONE_HALF<float32_t> ? state * Math::TWO<float32_t> -Math::ONE<float32_t> : Math::ZERO<float32_t>;
					break;
				case GamepadKeyFlag::FLIP:
					state = Math::ONE<float32_t> - state;
					break;
				default:
					break;
				}
			}
		}

		return state;
	}


	GenericGamepad::GenericGamepad(const DeviceInfo& info, IGenericGamepadDriver& driver, const GamepadKeyMapper* keyMapper) :
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_driver(driver),
		_polling(false),
		_inputState(nullptr),
		_oldInputState(nullptr),
		_inputBuffer(nullptr),
		_outputState(nullptr),
		_outputBuffer(nullptr),
		_outputFlags(0),
		_needOutput(false) {
		if (auto inputLength = _driver->getInputLength(); inputLength) {
			_inputState = new uint8_t[inputLength];
			_oldInputState = new uint8_t[inputLength];
			_inputBuffer = new uint8_t[inputLength];
			memset(_inputState, 0, inputLength);
			memset(_oldInputState, 0, inputLength);
			memset(_inputBuffer, 0, inputLength);
		}

		_outputLength = _driver->getOutputLength();
		if (_outputLength) {
			_outputState = new uint8_t[_outputLength];
			_outputBuffer = new uint8_t[_outputLength];
		}

		_driver->init(_inputState, _outputState);
		_driver->setKeyMapper(_keyMapper, keyMapper);
	}

	GenericGamepad::~GenericGamepad() {
		if (_inputState) {
			delete[] _inputBuffer;
			delete[] _oldInputState;
			delete[] _inputState;
		}

		if (_outputState) {
			delete[] _outputState;
			delete[] _outputBuffer;
		}
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> GenericGamepad::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& GenericGamepad::getInfo() const {
		return _info;
	}

	DeviceState::CountType GenericGamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		using namespace srk::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				switch ((GamepadVirtualKeyCode)code) {
				case GamepadVirtualKeyCode::L_STICK:
				case GamepadVirtualKeyCode::R_STICK:
				case GamepadVirtualKeyCode::DPAD:
				{
					GamepadKeyCodeAndFlags mappingVals[4];
					auto vk = GamepadVirtualKeyCode::L_STICK_X_LEFT + ((code - (DeviceState::CodeType)GamepadVirtualKeyCode::L_STICK) << 2);
					_keyMapper.get(vk, 4, mappingVals);

					float32_t xl, xr, yd, yu;
					{
						std::shared_lock lock(_inputMutex);

						if (!_driver->isStateReady(_inputState)) return 0;
						xl = _driver->readDataFromInputState(_inputState, mappingVals[0], Math::ZERO<float32_t>);
						xr = _driver->readDataFromInputState(_inputState, mappingVals[1], Math::ZERO<float32_t>);
						yd = _driver->readDataFromInputState(_inputState, mappingVals[2], Math::ZERO<float32_t>);
						yu = _driver->readDataFromInputState(_inputState, mappingVals[3], Math::ZERO<float32_t>);
					}

					return translate(_normalizeStick(xl, xr), _normalizeStick(yd, yu), _deadZone.get((GamepadVirtualKeyCode)code), (DeviceStateValue*)values, count);
				}
				default:
				{
					if ((code >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) ||
						(code >= GamepadVirtualKeyCode::BUTTON_START && code <= GamepadVirtualKeyCode::BUTTON_END)) {
						auto vk = (GamepadVirtualKeyCode)code;
						auto cf = _keyMapper.get(vk);

						float32_t val;
						{
							std::shared_lock lock(_inputMutex);

							if (!_driver->isStateReady(_inputState)) return 0;
							val = _driver->readDataFromInputState(_inputState, cf, Math::ZERO<float32_t>);
						}

						((DeviceStateValue*)values)[0] = translate(val, _deadZone.get(vk));

						return 1;
					}

					return 0;
				}
				}
			}

			return 0;
		}
		case DeviceStateType::KEY_MAPPER:
		{
			if (values && count) {
				*((GamepadKeyMapper*)values) = _keyMapper;

				return 1;
			}

			return 0;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				DeviceState::CountType c = 1;

				auto dz = _deadZone.get((GamepadVirtualKeyCode)code);
				((DeviceStateValue*)values)[0] = dz[0];
				if (count > 1) ((DeviceStateValue*)values)[c++] = dz[1];

				return c;
			}

			return 0;
		}
		default:
		{
			std::shared_lock lock(_inputMutex, std::defer_lock);

			return _driver->customGetState(type, code, values, count, _inputState, &lock, 
				[](void* custom) {
					auto lock = (std::shared_lock<std::shared_mutex>*)custom;
					lock->lock();
				},
				[](void* custom) {
					auto lock = (std::shared_lock<std::shared_mutex>*)custom;
					lock->unlock();
				});
		}
		}
	}

	DeviceState::CountType GenericGamepad::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::KEY_MAPPER:
		{
			if (!count) values = nullptr;

			_driver->setKeyMapper(_keyMapper, (const GamepadKeyMapper*)values);

			return 1;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (!count) values = nullptr;

			if (values) {
				DeviceState::CountType c = 1;

				Vec2<DeviceStateValue> dz;
				dz[0] = ((DeviceStateValue*)values)[0];
				if (count > 1) dz[c++] = ((DeviceStateValue*)values)[1];

				_setDeadZone((GamepadVirtualKeyCode)code, &dz);

				return c;
			} else {
				_setDeadZone((GamepadVirtualKeyCode)code, nullptr);
				return 1;
			}

			return 0;
		}
		default:
		{
			void* custom[2];
			custom[1] = this;

			std::unique_lock lock(_outputMutex, std::defer_lock);

			custom[0] = &lock;

			return _driver->customSetState(type, code, values, count, _outputState, custom,
				[](void* custom) {
				auto lock = (std::unique_lock<std::shared_mutex>*)((void**)custom)[0];
				lock->lock();
			},
				[](void* custom) {
				auto self = (GenericGamepad*)((void**)custom)[1];

				self->_outputFlags.fetch_or(OUTPUT_DIRTY);

				auto lock = (std::unique_lock<std::shared_mutex>*)((void**)custom)[0];
				lock->unlock();
			});
		}
		}
	}

	void GenericGamepad::poll(bool dispatchEvent) {
		if (_polling.exchange(true, std::memory_order::acquire)) return;

		_doInput(dispatchEvent);
		_doOutput();

		_polling.store(false, std::memory_order::release);
	}

	void GenericGamepad::_doInput(bool dispatchEvent) {
		using namespace srk::enum_operators;

		if (!_driver->readStateFromDevice(_inputBuffer)) return;

		if (!dispatchEvent) {
			_switchInputData();

			return;
		}

		GamepadKeyMapper keyMapper(_keyMapper);

		_switchInputData();

		if (!_driver->isStateReady(_oldInputState)) return;

		{
			GamepadKeyCodeAndFlags mappingVals[4];
			DeviceStateValue oldVals[2], newVals[2];
			for (size_t i = 0; i < 3; ++i) {
				keyMapper.get(GamepadVirtualKeyCode::L_STICK_X_LEFT + (i << 2), 4, mappingVals);
				auto vk = GamepadVirtualKeyCode::L_STICK + i;

				auto dz = _deadZone.get(vk);

				translate(_normalizeStick(_driver->readDataFromInputState(_oldInputState, mappingVals[0], Math::ZERO<float32_t>), _driver->readDataFromInputState(_oldInputState, mappingVals[1], Math::ZERO<float32_t>)),
					_normalizeStick(_driver->readDataFromInputState(_oldInputState, mappingVals[2], Math::ZERO<float32_t>), _driver->readDataFromInputState(_oldInputState, mappingVals[3], Math::ZERO<float32_t>)), dz, oldVals, 2);
				translate(_normalizeStick(_driver->readDataFromInputState(_inputState, mappingVals[0], Math::ZERO<float32_t>), _driver->readDataFromInputState(_inputState, mappingVals[1], Math::ZERO<float32_t>)),
					_normalizeStick(_driver->readDataFromInputState(_inputState, mappingVals[2], Math::ZERO<float32_t>), _driver->readDataFromInputState(_inputState, mappingVals[3], Math::ZERO<float32_t>)), dz, newVals, 2);

				if (oldVals[0] != newVals[0] || oldVals[1] != newVals[1]) {
					DeviceState ds = { (DeviceState::CodeType)vk, 2, newVals };
					_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
				}
			}
		}

		keyMapper.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCodeAndFlags cf) {
			if (vk >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && vk <= GamepadVirtualKeyCode::AXIS_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = translate(_driver->readDataFromInputState(_inputState, cf, Math::ZERO<float32_t>), dz); newVal != translate(_driver->readDataFromInputState(_oldInputState, cf, Math::ZERO<float32_t>), dz)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &ds);
				}
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = _driver->readDataFromInputState(_inputState, cf, Math::ZERO<float32_t>); newVal != _driver->readDataFromInputState(_oldInputState, cf, Math::ZERO<float32_t>)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, val > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
				}
			}
		});

		_driver->customDispatch(_oldInputState, _inputState, this, [](DeviceEvent evt, void* data, void* custom) {
			auto self = (GenericGamepad*)custom;
			self->_eventDispatcher->dispatchEvent(self, evt, data);
		});
	}

	void GenericGamepad::_doOutput() {
		auto expected = OUTPUT_DIRTY;
		if (_outputFlags.compare_exchange_strong(expected, OUTPUT_WRITING, std::memory_order::release, std::memory_order::relaxed)) {
			_needOutput = true;

			std::shared_lock lock(_outputMutex);

			memcpy(_outputBuffer, _outputState, _outputLength);
		}

		if (_needOutput && _driver->writeStateToDevice(_outputBuffer)) {
			_needOutput = false;
			_outputFlags.fetch_and(~OUTPUT_WRITING);
		}
	}

	void GenericGamepad::_setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone) {
		if (deadZone) {
			auto& dzVal = *deadZone;

			Math::clamp(dzVal.data, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>);

			if (dzVal[1] < dzVal[0]) {
				auto tmp = dzVal[0];
				dzVal[0] = dzVal[1];
				dzVal[1] = tmp;
			}

			_deadZone.set(keyCode, dzVal);
		} else {
			_deadZone.remove(keyCode);
		}
	}

	void GenericGamepad::_switchInputData() {
		std::scoped_lock lock(_inputMutex);

		auto tmp = _oldInputState;
		_oldInputState = _inputState;
		_inputState = _inputBuffer;
		_inputBuffer = tmp;
	}
}