#include "GenericMouse.h"

namespace srk::modules::inputs {
	GenericMouseBuffer::GenericMouseBuffer() :
		wheel(0.f) {
		memset(buttons, 0, sizeof(buttons));
	}

	GenericMouseBuffer::GenericMouseBuffer(nullptr_t) :
		pos(nullptr) {
	}


	GenericMouse::GenericMouse(const DeviceInfo& info, IGenericMouseDriver& driver) : GenericMouseBase(info, driver) {
		_curInputBuffer = &_inputBuffers[0];
		_prevInputBuffer = &_inputBuffers[1];
		_readDeviceInputBuffer = &_inputBuffers[2];
	}

	GenericMouse::~GenericMouse() {
		close();
	}

	DeviceState::CountType GenericMouse::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		using namespace srk::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				switch ((MouseVirtualKeyCode)code) {
				case MouseVirtualKeyCode::POSITION:
				{
					Vec2f32 pos(nullptr);

					{
						std::shared_lock lock(_inputMutex);

						pos = _curInputBuffer->pos;
					}

					((DeviceStateValue*)values)[0] = pos[0];
					DeviceState::CountType c = 1;
					if (count > 1) ((DeviceStateValue*)values)[c++] = pos[1];

					return c;
				}
				case MouseVirtualKeyCode::WHEEL:
					return 0;
				default:
				{
					if (code >= MouseVirtualKeyCode::BUTTON_START && code <= MouseVirtualKeyCode::BUTTON_END) {
						((DeviceStateValue*)values)[0] = _curInputBuffer->getButton((MouseVirtualKeyCode)code, _inputMutex) ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

						return 1;
					}

					break;
				}
				}
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType GenericMouse::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		return 0;
	}

	std::optional<bool> GenericMouse::_readFromDevice() {
		return _driver->readFromDevice(*_readDeviceInputBuffer);
	}

	bool GenericMouse::_writeToDevice() {
		return true;
	}

	bool GenericMouse::_doInput() {
		if (_curInputBuffer->pos != _prevInputBuffer->pos) {
			DeviceStateValue value[] = { (DeviceStateValue)_curInputBuffer->pos[0], (DeviceStateValue)_curInputBuffer->pos [1]};
			DeviceState k = { (DeviceState::CodeType)MouseVirtualKeyCode::POSITION, 2, value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		if (_curInputBuffer->wheel != 0.f) {
			DeviceStateValue value = _curInputBuffer->wheel;
			DeviceState k = { (DeviceState::CodeType)MouseVirtualKeyCode::WHEEL, 1, &value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}

		auto& cur = _curInputBuffer->buttons;
		auto& old = _prevInputBuffer->buttons;
		for (size_t i = 0; i < sizeof(GenericMouseBuffer::ButtonData); ++i) {
			if (cur[i] != old[i]) {
				for (size_t j = 0; j < 8; ++j) {
					auto mask = 1 << j;
					auto curVal = cur[i] & mask;
					auto oldVal = old[i] & mask;
					if (curVal != oldVal) {
						DeviceStateValue value = curVal ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;

						DeviceState k = { (DeviceState::CodeType)((i << 3) + j) + (DeviceState::CodeType)MouseVirtualKeyCode::BUTTON_START, 1, &value };
						_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
					}
				}
			}
		}

		return true;
	}

	void SRK_CALL GenericMouse::_closeDevice() {
		_driver->close();
		_driver.reset();
	}
}