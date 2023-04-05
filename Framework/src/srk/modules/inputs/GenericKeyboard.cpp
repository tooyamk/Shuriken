#include "GenericKeyboard.h"

namespace srk::modules::inputs {
	GenericKeyboardBuffer::GenericKeyboardBuffer() {
		memset(data, 0, sizeof(data));
	}

	GenericKeyboardBuffer::GenericKeyboardBuffer(nullptr_t) {
	}


	GenericKeyboard::GenericKeyboard(const DeviceInfo& info, IGenericKeyboardDriver& driver) : GenericKeyboardBase(info, driver) {
		_curInputBuffer = &_inputBuffers[0];
		_prevInputBuffer = &_inputBuffers[1];
		_readDeviceInputBuffer = &_inputBuffers[2];
	}

	GenericKeyboard::~GenericKeyboard() {
		close();
	}

	DeviceState::CountType GenericKeyboard::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_inputMutex);

				((DeviceStateValue*)values)[0] = _curInputBuffer->get((KeyboardVirtualKeyCode)code) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
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

	std::optional<bool> GenericKeyboard::_readFromDevice() {
		return _driver->readFromDevice(*_readDeviceInputBuffer);
	}

	bool GenericKeyboard::_writeToDevice() {
		return true;
	}

	bool GenericKeyboard::_doInput() {
		auto& cur = _curInputBuffer->data;
		auto& old = _prevInputBuffer->data;
		for (size_t i = 0; i < sizeof(GenericKeyboardBuffer::Data); ++i) {
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

	void SRK_CALL GenericKeyboard::_closeDevice() {
		_driver->close();
		_driver.reset();
	}
}