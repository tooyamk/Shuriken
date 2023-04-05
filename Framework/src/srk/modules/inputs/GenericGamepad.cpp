#include "GenericGamepad.h"

namespace srk::modules::inputs {
	GenericGamepad::GenericGamepad(const DeviceInfo& info, IGenericGamepadDriver& driver, const GamepadKeyMapper* keyMapper) : GenericGamepadBase(info, driver),
		_buffers(nullptr) {
		auto inputBufferLength = _driver->getInputBufferLength();
		_outputBufferLength = _driver->getOutputBufferLength();

		auto size = inputBufferLength * 3 + _outputBufferLength * 2;
		if (size) {
			_buffers = new uint8_t[size];
			memset(_buffers, 0, size);

			auto offset = 0;
			if (inputBufferLength) {
				_curInputBuffer = _buffers;
				offset += inputBufferLength;

				_prevInputBuffer = _buffers + offset;
				offset += inputBufferLength;

				_readDeviceInputBuffer = _buffers + offset;
				offset += inputBufferLength;


			}

			if (_outputBufferLength) {
				_outputBuffer = _buffers + offset;
				offset += _outputBufferLength;

				_writingOutputBuffer = _buffers + offset;
			}
		}

		_driver->init(_curInputBuffer, _outputBuffer);
		_driver->setKeyMapper(_keyMapper, keyMapper);
	}

	GenericGamepad::~GenericGamepad() {
		if (_buffers) delete[] _buffers;
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

						if (!_driver->isBufferReady(_curInputBuffer)) return 0;
						
						xl = _readFromInputBuffer(_curInputBuffer, mappingVals[0], Math::ONE_HALF<DeviceStateValue>);
						xr = _readFromInputBuffer(_curInputBuffer, mappingVals[1], Math::ONE_HALF<DeviceStateValue>);
						yd = _readFromInputBuffer(_curInputBuffer, mappingVals[2], Math::ONE_HALF<DeviceStateValue>);
						yu = _readFromInputBuffer(_curInputBuffer, mappingVals[3], Math::ONE_HALF<DeviceStateValue>);
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

							if (!_driver->isBufferReady(_curInputBuffer)) return 0;
							val = _readFromInputBuffer(_curInputBuffer, cf, Math::NEGATIVE_ONE<DeviceStateValue>);
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

			return _driver->customGetState(type, code, values, count, _curInputBuffer, &lock,
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

			return _driver->customSetState(type, code, values, count, _outputBuffer, custom,
				[](void* custom) {
				auto lock = (std::unique_lock<std::shared_mutex>*)((void**)custom)[0];
				lock->lock();
			},
				[](void* custom) {
				auto self = (GenericGamepad*)((void**)custom)[1];

				self->_outputFlag.fetch_or(OUTPUT_FLAG_DIRTY);

				auto lock = (std::unique_lock<std::shared_mutex>*)((void**)custom)[0];
				lock->unlock();
			});
		}
		}
	}

	std::optional<bool> GenericGamepad::_readFromDevice() {
		return _driver->readFromDevice(_readDeviceInputBuffer);
	}

	bool GenericGamepad::_writeToDevice() {
		return _driver->writeToDevice(_writingOutputBuffer);
	}

	bool GenericGamepad::_doInput() {
		using namespace srk::enum_operators;

		GamepadKeyMapper keyMapper(_keyMapper);

		if (!_driver->isBufferReady(_prevInputBuffer)) return true;

		_doInputMove(keyMapper, GamepadVirtualKeyCode::L_STICK, GamepadVirtualKeyCode::L_STICK_X_LEFT);
		_doInputMove(keyMapper, GamepadVirtualKeyCode::R_STICK, GamepadVirtualKeyCode::R_STICK_X_LEFT);
		_doInputMove(keyMapper, GamepadVirtualKeyCode::DPAD, GamepadVirtualKeyCode::DPAD_LEFT);

		keyMapper.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCodeAndFlags cf) {
			if (vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = translate(_readFromInputBuffer(_curInputBuffer, cf, Math::ZERO<DeviceStateValue>), dz); newVal != translate(_readFromInputBuffer(_prevInputBuffer, cf, Math::ZERO<DeviceStateValue>), dz)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &ds);
				}
			} else if (vk >= GamepadVirtualKeyCode::UNDEFINED_HAT_1 && vk <= GamepadVirtualKeyCode::UNDEFINED_HAT_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = translate(_readFromInputBuffer(_curInputBuffer, cf, Math::NEGATIVE_ONE<DeviceStateValue>), dz); newVal != translate(_readFromInputBuffer(_prevInputBuffer, cf, Math::NEGATIVE_ONE<DeviceStateValue>), dz)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &ds);
				}
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				auto dz = _deadZone.get(vk);
				if (auto newVal = _readFromInputBuffer(_curInputBuffer, cf, Math::ZERO<DeviceStateValue>); newVal != _readFromInputBuffer(_prevInputBuffer, cf, Math::ZERO<DeviceStateValue>)) {
					DeviceStateValue val = newVal;
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &val };
					_eventDispatcher->dispatchEvent(this, val > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
				}
			}
		});

		_driver->customDispatch(_prevInputBuffer, _curInputBuffer, this, [](DeviceEvent evt, void* data, void* custom) {
			auto self = (GenericGamepad*)custom;
			self->_eventDispatcher->dispatchEvent(self, evt, data);
		});

		return true;
	}

	void SRK_CALL GenericGamepad::_closeDevice() {
		_driver->close();
	}

	void GenericGamepad::_doInputMove(const GamepadKeyMapper& mapper, GamepadVirtualKeyCode combined, GamepadVirtualKeyCode separatedBegin) {
		GamepadKeyCodeAndFlags mappingVals[4];
		DeviceStateValue oldVals[2], newVals[2];
		
		mapper.get(separatedBegin, 4, mappingVals);

		auto dz = _deadZone.get(combined);

		translate(_normalizeStick(_readFromInputBuffer(_prevInputBuffer, mappingVals[0], Math::ONE_HALF<DeviceStateValue>), _readFromInputBuffer(_prevInputBuffer, mappingVals[1], Math::ONE_HALF<DeviceStateValue>)),
			_normalizeStick(_readFromInputBuffer(_prevInputBuffer, mappingVals[2], Math::ONE_HALF<DeviceStateValue>), _readFromInputBuffer(_prevInputBuffer, mappingVals[3], Math::ONE_HALF<DeviceStateValue>)), dz, oldVals, 2);
		translate(_normalizeStick(_readFromInputBuffer(_curInputBuffer, mappingVals[0], Math::ONE_HALF<DeviceStateValue>), _readFromInputBuffer(_curInputBuffer, mappingVals[1], Math::ONE_HALF<DeviceStateValue>)),
			_normalizeStick(_readFromInputBuffer(_curInputBuffer, mappingVals[2], Math::ONE_HALF<DeviceStateValue>), _readFromInputBuffer(_curInputBuffer, mappingVals[3], Math::ONE_HALF<DeviceStateValue>)), dz, newVals, 2);

		if (oldVals[0] != newVals[0] || oldVals[1] != newVals[1]) {
			DeviceState ds = { (DeviceState::CodeType)combined, 2, newVals };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
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
}