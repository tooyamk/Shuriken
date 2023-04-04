#include "GenericKeyboard.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs {
	GenericKeyboard::Buffer::Buffer() {
		memset(data, 0, sizeof(data));
	}

	GenericKeyboard::Buffer::Buffer(nullptr_t) {
	}


	GenericKeyboard::GenericKeyboard(const DeviceInfo& info, IGenericKeyboardDriver& driver) :
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_driver(driver),
		_polling(false),
		_closed(false) {
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
			if (values && count) {
				std::shared_lock lock(_inputMutex);

				((DeviceStateValue*)values)[0] = _inputBuffer->get((KeyboardVirtualKeyCode)code) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
				return 1;
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

	DevicePollResult GenericKeyboard::poll(bool dispatchEvent) {
		using namespace srk::enum_operators;

		if (_closed) return DevicePollResult::CLOSED;
		if (_polling.exchange(true, std::memory_order::acquire)) return DevicePollResult::EMPTY;

		auto rst = DevicePollResult::ACQUIRED;

		if (_doInput(dispatchEvent)) rst |= DevicePollResult::INPUT_COMPLETE;
		rst |= DevicePollResult::OUTPUT__COMPLETE;

		_polling.store(false, std::memory_order::release);

		return rst;
	}

	void GenericKeyboard::close() {
		_closed = true;
	}

	bool GenericKeyboard::_doInput(bool dispatchEvent) {
		auto opt = _driver->readStateFromDevice(*_readInputBuffer);
		if (!opt) return false;
		if (!(*opt)) return true;

		if (!dispatchEvent) {
			_switchInputData();

			return true;
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

						DeviceState k = { (DeviceState::CodeType)((i << 3) + j) + (DeviceState::CodeType)KeyboardVirtualKeyCode::DEFINED_START, 1, &value };
						_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
					}
				}
			}
		}

		return true;
	}

	void GenericKeyboard::_switchInputData() {
		std::scoped_lock lock(_inputMutex);

		auto tmp = _oldInputBuffer;
		_oldInputBuffer = _inputBuffer;
		_inputBuffer = _readInputBuffer;
		_readInputBuffer = tmp;
	}
}