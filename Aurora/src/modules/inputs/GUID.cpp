#include "GUID.h"

namespace aurora::modules::inputs {
	GUID::GUID() :
		_data(nullptr),
		_len(0) {
	}

	GUID::GUID(const GUID& value) :
		_data(new i8[value._len]),
		_len(value._len) {
		memcpy(_data, value._data, _len);
	}

	GUID::GUID(GUID&& value) :
		_data(value._data),
		_len(value._len) {
		value._data = nullptr;
	}

	GUID::~GUID() {
		if (_data) delete[] _data;
	}

	void GUID::set(const i8* data, ui32 len) {
		if (_len != len) {
			_len = len;
			delete[] _data;
			_data = new i8[_len];
		}

		memcpy(_data, data, _len);
	}

	bool GUID::isEqual(const i8* data, ui32 len) const {
		if (_len == len) {
			for (ui32 i = 0; i < _len; ++i) {
				if (_data[i] != data[i]) return false;
			}
			return true;
		}
		return false;
	}
}