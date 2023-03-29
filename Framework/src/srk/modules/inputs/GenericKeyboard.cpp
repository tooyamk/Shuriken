#include "GenericKeyboard.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs {
	GenericKeyboard::GenericKeyboard(const DeviceInfo& info, IGenericKeyboardDriver& driver) :
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_driver(driver),
		_polling(false) {
		memset(_inputBuffers, 0, sizeof(_inputBuffers));
		_inputBuffer = &_inputBuffers[0];
		_oldInputBuffer = &_inputBuffers[1];
		_readInputBuffer = &_inputBuffers[2];
	}

	GenericKeyboard::~GenericKeyboard() {
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> GenericKeyboard::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& GenericKeyboard::getInfo() const {
		return _info;
	}

	DeviceState::CountType GenericKeyboard::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count && code < (sizeof(Buffer) << 3)) {
				std::shared_lock lock(_inputMutex);

				switch ((KeyboardVirtualKeyCode)code) {
				case KeyboardVirtualKeyCode::SHIFT:
					((DeviceStateValue*)values)[0] = _inputBuffer->get(KeyboardVirtualKeyCode::L_SHIFT) || _inputBuffer->get(KeyboardVirtualKeyCode::R_SHIFT) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				case KeyboardVirtualKeyCode::CTRL:
					((DeviceStateValue*)values)[0] = _inputBuffer->get(KeyboardVirtualKeyCode::L_CTRL) || _inputBuffer->get(KeyboardVirtualKeyCode::R_CTRL) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				case KeyboardVirtualKeyCode::ALT:
					((DeviceStateValue*)values)[0] = _inputBuffer->get(KeyboardVirtualKeyCode::L_ALT) || _inputBuffer->get(KeyboardVirtualKeyCode::R_ALT) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				default:
					((DeviceStateValue*)values)[0] = _inputBuffer->get((KeyboardVirtualKeyCode)code) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
					return 1;
				}
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType GenericKeyboard::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		return 0;
	}

	void GenericKeyboard::poll(bool dispatchEvent) {
		if (_polling.exchange(true, std::memory_order::acquire)) return;

		_doInput(dispatchEvent);

		_polling.store(false, std::memory_order::release);
	}

	void GenericKeyboard::_doInput(bool dispatchEvent) {
		if (!_driver->readStateFromDevice(*_readInputBuffer)) return;

		if (!dispatchEvent) {
			_switchInputData();

			return;
		}

		_switchInputData();

		auto& cur = _inputBuffer->data;
		auto& old = _oldInputBuffer->data;
		for (size_t i = 0; i < sizeof(Buffer::Data); ++i) {
			if (cur[i] != old[i]) {
				for (size_t j = 0; j < 8; ++j) {
					auto mask = 1 << j;
					auto curVal = cur[i] & mask;
					auto oldVal = old[i] & mask;
					if (curVal != oldVal) {
						DeviceStateValue value = curVal ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

						DeviceState k = { (i << 3) + j, 1, &value };
						_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
					}
				}
			}
		}
	}

	void GenericKeyboard::_switchInputData() {
		std::scoped_lock lock(_inputMutex);

		auto tmp = _oldInputBuffer;
		_oldInputBuffer = _inputBuffer;
		_inputBuffer = _readInputBuffer;
		_readInputBuffer = tmp;
	}
}