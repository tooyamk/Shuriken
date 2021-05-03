#include "GenericGamepad.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs {
	float32_t IGenericGamepadDriver::translate(float32_t state, GamepadKeyFlag flags) {
		using namespace aurora::enum_operators;

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


	GenericGamepad::GenericGamepad(const DeviceInfo& info, IGenericGamepadDriver& driver) :
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_driver(driver),
		_inputState(nullptr),
		_oldInputState(nullptr),
		_newInputState(nullptr),
		_outputState(nullptr),
		_outputBuffer(nullptr),
		_outputDirty(false),
		_needOutput(false) {
		_inputLength = _driver->getInputLength();
		if (_inputLength) {
			_inputState = new uint8_t[_inputLength];
			_oldInputState = new uint8_t[_inputLength];
			_newInputState = new uint8_t[_inputLength];
			memset(_inputState, 0, _inputLength);
			memset(_oldInputState, 0, _inputLength);
			memset(_newInputState, 0, _inputLength);
		}

		_outputLength = _driver->getOutputLength();
		if (_outputLength) {
			_outputState = new uint8_t[_outputLength];
			_outputBuffer = new uint8_t[_outputLength];
		}

		_driver->init(_inputState, _outputState);
		_driver->setKeyMapping(_keyMapping, nullptr);
	}

	GenericGamepad::~GenericGamepad() {
		if (_inputState) {
			delete[] _newInputState;
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
		using namespace aurora::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				switch ((GamepadVirtualKeyCode)code) {
				case GamepadVirtualKeyCode::L_STICK:
				case GamepadVirtualKeyCode::R_STICK:
				{
					GamepadKeyCodeAndFlags mappingVals[2];
					auto vk = code == GamepadVirtualKeyCode::L_STICK ? GamepadVirtualKeyCode::L_STICK_X : GamepadVirtualKeyCode::R_STICK_X;
					{
						std::shared_lock lock(_keyMappingMutex);

						_keyMapping.get(vk, 2, mappingVals);
					}

					float32_t x, y;
					{
						std::shared_lock lock(_inputMutex);

						x = _driver->readDataFromInputState(_inputState, mappingVals[0], Math::ONE_HALF<float32_t>);
						y = _driver->readDataFromInputState(_inputState, mappingVals[1], Math::ONE_HALF<float32_t>);
					}

					return translate(_normalizeStick(x), _normalizeStick(y), _getDeadZone((GamepadVirtualKeyCode)code), (DeviceStateValue*)values, count);
				}
				case GamepadVirtualKeyCode::DPAD:
				{
					std::shared_lock lock(_inputMutex);

					((DeviceStateValue*)values)[0] = _driver->readDpadDataFromInputState(_inputState);

					return 1;
				}
				default:
				{
					if ((code >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) ||
						(code >= GamepadVirtualKeyCode::BUTTON_START && code <= GamepadVirtualKeyCode::BUTTON_END)) {
						auto vk = (GamepadVirtualKeyCode)code;

						GamepadKeyCodeAndFlags cf;
						{
							std::shared_lock lock(_keyMappingMutex);

							cf = _keyMapping.get(vk);
						}

						float32_t val;
						{
							std::shared_lock lock(_inputMutex);

							val = _driver->readDataFromInputState(_inputState, cf, Math::ZERO<float32_t>);
						}

						((DeviceStateValue*)values)[0] = translate(val, _getDeadZone(vk));

						return 1;
					}

					return 0;
				}
				}
			}

			return 0;
		}
		case DeviceStateType::KEY_MAPPING:
		{
			if (values && count) {
				std::shared_lock lock(_keyMappingMutex);

				*((GamepadKeyMapping*)values) = _keyMapping;

				return 1;
			}

			return 0;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				DeviceState::CountType c = 1;

				auto dz = _getDeadZone((GamepadVirtualKeyCode)code);
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
		case DeviceStateType::KEY_MAPPING:
		{
			if (!count) values = nullptr;

			{
				std::scoped_lock lock(_keyMappingMutex);

				_driver->setKeyMapping(_keyMapping, (const GamepadKeyMapping*)values);
			}

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
			return _driver->customSetState(type, code, values, count, this, [](const void* data, size_t dataLength, size_t outputStateOffset, void* custom) {
				auto self = (GenericGamepad*)custom;

				std::scoped_lock lock(self->_outputMutex);

				memcpy(self->_outputState + outputStateOffset, data, dataLength);
				self->_outputDirty = true;
			});
		}
	}

	void GenericGamepad::poll(bool dispatchEvent) {
		std::scoped_lock lock(_pollMutex);

		_doInput(dispatchEvent);
		_doOutput();
	}

	void GenericGamepad::_doInput(bool dispatchEvent) {
		using namespace aurora::enum_operators;

		if (!_driver->readStateFromDevice(_newInputState)) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_inputMutex);

			memcpy(_inputState, _newInputState, _inputLength);

			return;
		}

		GamepadKeyMapping keyMapping(NO_INIT);
		{
			std::shared_lock lock(_keyMappingMutex);

			keyMapping = _keyMapping;
		}
		{
			std::scoped_lock lock(_inputMutex);

			memcpy(_oldInputState, _inputState, _inputLength);
			memcpy(_inputState, _newInputState, _inputLength);
		}

		GamepadKeyCodeAndFlags mappingVals[4];
		keyMapping.get(GamepadVirtualKeyCode::L_STICK_X, 4, mappingVals);
		for (size_t i = 0; i < 2; ++i) {
			auto idx = i << 1;
			auto vk = GamepadVirtualKeyCode::L_STICK + i;

			auto dz = _getDeadZone(vk);

			DeviceStateValue oldVals[2], newVals[2];
			translate(_normalizeStick(_driver->readDataFromInputState(_oldInputState, mappingVals[idx], Math::ONE_HALF<float32_t>)), 
				_normalizeStick(_driver->readDataFromInputState(_oldInputState, mappingVals[idx + 1], Math::ONE_HALF<float32_t>)), dz, oldVals, 2);
			translate(_normalizeStick(_driver->readDataFromInputState(_newInputState, mappingVals[idx], Math::ONE_HALF<float32_t>)), 
				_normalizeStick(_driver->readDataFromInputState(_newInputState, mappingVals[idx + 1], Math::ONE_HALF<float32_t>)), dz, newVals, 2);

			if (oldVals[0] != newVals[0] || oldVals[1] != newVals[1]) {
				DeviceState ds = { (DeviceState::CodeType)vk, 2, newVals };
				_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
			}
		}

		keyMapping.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCodeAndFlags cf) {
			if (vk >= GamepadVirtualKeyCode::L_TRIGGER && vk <= GamepadVirtualKeyCode::AXIS_END) {
				auto dz = _getDeadZone(vk);
				if (auto newVal = translate(_driver->readDataFromInputState(_newInputState, cf, Math::ZERO<float32_t>), dz); newVal != translate(_driver->readDataFromInputState(_oldInputState, cf, Math::ZERO<float32_t>), dz)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &ds);
				}
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				auto dz = _getDeadZone(vk);
				if (auto newVal = _driver->readDataFromInputState(_newInputState, cf, Math::ZERO<float32_t>); newVal != _driver->readDataFromInputState(_oldInputState, cf, Math::ZERO<float32_t>)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, val > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
				}
			}
		});

		if (auto newVal = _driver->readDpadDataFromInputState(_newInputState); newVal != _driver->readDpadDataFromInputState(_oldInputState)) {
			DeviceStateValue val = newVal;
			DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &val };
			_eventDispatcher->dispatchEvent(this, val >= Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
		}

		_driver->customDispatch(_oldInputState, _newInputState, this, [](DeviceEvent evt, void* data, void* custom) {
			auto self = (GenericGamepad*)custom;
			self->_eventDispatcher->dispatchEvent(self, evt, data);
		});
	}

	void GenericGamepad::_doOutput() {
		if (_outputDirty.exchange(false)) {
			_needOutput = true;

			std::shared_lock lock(_outputMutex);

			memcpy(_outputBuffer, _outputState, _outputLength);
		}

		if (_needOutput && _driver->writeStateToDevice(_outputBuffer)) _needOutput = false;
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

			std::scoped_lock lock(_deadZoneMutex);

			_deadZone.insert_or_assign(keyCode, dzVal);
		} else {
			std::scoped_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(keyCode); itr != _deadZone.end()) _deadZone.erase(itr);
		}
	}
}