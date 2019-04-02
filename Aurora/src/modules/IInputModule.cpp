#include "IInputModule.h"
#include "events/IEventDispatcher.h"

namespace aurora::modules {
	InputDeviceGUID::InputDeviceGUID() :
		_data(nullptr),
		_len(0) {
	}

	InputDeviceGUID::InputDeviceGUID(const InputDeviceGUID& value) :
		_data(new ui8[value._len]),
		_len(value._len) {
		memcpy(_data, value._data, _len);
	}

	InputDeviceGUID::InputDeviceGUID(InputDeviceGUID&& value) :
		_data(value._data),
		_len(value._len) {
		value._data = nullptr;
	}

	InputDeviceGUID::~InputDeviceGUID() {
		if (_data) delete[] _data;
	}

	void InputDeviceGUID::set(ui8* data, ui32 len) {
		if (_len != len) {
			_len = len;
			delete[] _data;
			_data = new ui8[_len];
		}

		memcpy(_data, data, _len);
	}

	bool InputDeviceGUID::isEqual(ui8* data, ui32 len) const {
		if (_len == len) {
			for (ui32 i = 0; i < _len; ++i) {
				if (_data[i] != data[i]) return false;
			}
			return true;
		}
		return false;
	}

	InputDeviceGUID& InputDeviceGUID::operator=(const InputDeviceGUID& value) {
		_len = value._len;
		_data = new ui8[_len];
		memcpy(_data, value._data, _len);

		return *this;
	}

	InputDeviceGUID& InputDeviceGUID::operator=(InputDeviceGUID&& value) {
		_data = value._data;
		_len = value._len;
		value._data = nullptr;

		return *this;
	}

	bool InputDeviceGUID::operator==(const InputDeviceGUID& right) const {
		return isEqual(right._data, right._len);
	}


	InputDeviceInfo::InputDeviceInfo() :
		type(0) {
	}

	InputDeviceInfo::InputDeviceInfo(const InputDeviceGUID& guid, ui32 type) :
		guid(guid),
		type(type) {
	}

	InputDeviceInfo::InputDeviceInfo(const InputDeviceInfo& value) : 
		guid(value.guid),
		type(value.type) {
	}

	InputDeviceInfo::InputDeviceInfo(InputDeviceInfo&& value) :
		guid(std::move(value.guid)),
		type(value.type) {
	}

	InputDeviceInfo& InputDeviceInfo::operator=(const InputDeviceInfo& value) {
		guid = value.guid;
		type = value.type;

		return *this;
	}

	InputDeviceInfo& InputDeviceInfo::operator=(InputDeviceInfo&& value) {
		guid = std::move(value.guid);
		type = value.type;

		return *this;
	}


	IInputDevice::~IInputDevice() {
	}


	IInputModule::IInputModule() {
	}

	IInputModule::~IInputModule() {
	}
}