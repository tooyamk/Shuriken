#include "GenericGamepad.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs {
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
						
						xl = _readDataFromInputState(_inputState, mappingVals[0], Math::ONE_HALF<DeviceStateValue>);
						xr = _readDataFromInputState(_inputState, mappingVals[1], Math::ONE_HALF<DeviceStateValue>);
						yd = _readDataFromInputState(_inputState, mappingVals[2], Math::ONE_HALF<DeviceStateValue>);
						yu = _readDataFromInputState(_inputState, mappingVals[3], Math::ONE_HALF<DeviceStateValue>);
					}

					return translate(_normalizeStick(xl, xr), _normalizeStick(yd, yu), _deadZone.get((GamepadVirtualKeyCode)code), (DeviceStateValue*)values, count);
				}
				default:
				{
					if (code >= GamepadVirtualKeyCode::SEPARATED_START && code <= GamepadVirtualKeyCode::SEPARATED_END) {
						auto vk = (GamepadVirtualKeyCode)code;
						auto cf = _keyMapper.get(vk);

						float32_t val;
						{
							std::shared_lock lock(_inputMutex);

							if (!_driver->isStateReady(_inputState)) return 0;
							val = _readDataFromInputState(_inputState, cf, Math::NEGATIVE_ONE<DeviceStateValue>);
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

		_doInputMove(keyMapper, GamepadVirtualKeyCode::L_STICK, GamepadVirtualKeyCode::L_STICK_X_LEFT);
		_doInputMove(keyMapper, GamepadVirtualKeyCode::R_STICK, GamepadVirtualKeyCode::R_STICK_X_LEFT);
		_doInputMove(keyMapper, GamepadVirtualKeyCode::DPAD, GamepadVirtualKeyCode::DPAD_LEFT);

		keyMapper.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCodeAndFlags cf) {
			if (vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = translate(_readDataFromInputState(_inputState, cf, Math::ZERO<DeviceStateValue>), dz); newVal != translate(_readDataFromInputState(_oldInputState, cf, Math::ZERO<DeviceStateValue>), dz)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &ds);
				}
			} else if (vk >= GamepadVirtualKeyCode::UNDEFINED_HAT_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_HAT_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = translate(_readDataFromInputState(_inputState, cf, Math::NEGATIVE_ONE<DeviceStateValue>), dz); newVal != translate(_readDataFromInputState(_oldInputState, cf, Math::NEGATIVE_ONE<DeviceStateValue>), dz)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &ds);
				}
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = _readDataFromInputState(_inputState, cf, Math::ZERO<DeviceStateValue>); newVal != _readDataFromInputState(_oldInputState, cf, Math::ZERO<DeviceStateValue>)) {
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

	void GenericGamepad::_doInputMove(const GamepadKeyMapper& mapper, GamepadVirtualKeyCode combined, GamepadVirtualKeyCode separatedBegin) {
		GamepadKeyCodeAndFlags mappingVals[4];
		DeviceStateValue oldVals[2], newVals[2];
		
		mapper.get(separatedBegin, 4, mappingVals);

		auto dz = _deadZone.get(combined);

		translate(_normalizeStick(_readDataFromInputState(_oldInputState, mappingVals[0], Math::ONE_HALF<DeviceStateValue>), _readDataFromInputState(_oldInputState, mappingVals[1], Math::ONE_HALF<DeviceStateValue>)),
			_normalizeStick(_readDataFromInputState(_oldInputState, mappingVals[2], Math::ONE_HALF<DeviceStateValue>), _readDataFromInputState(_oldInputState, mappingVals[3], Math::ONE_HALF<DeviceStateValue>)), dz, oldVals, 2);
		translate(_normalizeStick(_readDataFromInputState(_inputState, mappingVals[0], Math::ONE_HALF<DeviceStateValue>), _readDataFromInputState(_inputState, mappingVals[1], Math::ONE_HALF<DeviceStateValue>)),
			_normalizeStick(_readDataFromInputState(_inputState, mappingVals[2], Math::ONE_HALF<DeviceStateValue>), _readDataFromInputState(_inputState, mappingVals[3], Math::ONE_HALF<DeviceStateValue>)), dz, newVals, 2);

		if (oldVals[0] != newVals[0] || oldVals[1] != newVals[1]) {
			DeviceState ds = { (DeviceState::CodeType)combined, 2, newVals };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
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