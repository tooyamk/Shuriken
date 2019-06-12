#include "SerializableObject.h"

namespace aurora {
	SerializableObject::Array::Array() {
		ref();
	}

	SerializableObject::Array* SerializableObject::Array::copy() const {
		Array* arr = new Array();
		arr->value.resize(this->value.size());
		ui32 idx = 0;
		for (auto& e : this->value) arr->value[idx++].set(e, true);
		return arr;
	}

	bool SerializableObject::Array::isContentEqual(Array* data) const {
		auto size = this->value.size();
		if (size == data->value.size()) {
			for (ui32 i = 0; i < size; ++i) {
				if (!this->value[i].isContentEqual(data->value[i])) return false;
			}

			return true;
		} else {
			return false;
		}
	}


	SerializableObject::Map::Map() {
		ref();
	}

	SerializableObject::Map::Map(const SerializableObject& key, const SerializableObject& value) : Map() {
		this->value.emplace(key, value);
	}

	SerializableObject::Map* SerializableObject::Map::copy() const {
		Map* map = new Map();
		for (auto& itr : this->value) map->value.emplace(SerializableObject(itr.first, true), SerializableObject(itr.second, true));
		return map;
	}

	bool SerializableObject::Map::isContentEqual(Map* data) const {
		int size = this->value.size();
		if (size == data->value.size()) {
			for (auto& itr : this->value) {
				if (auto itr2 = data->value.find(itr.first); itr2 == data->value.end() || !itr.second.isContentEqual(itr2->second)) return false;
			}

			return true;
		} else {
			return false;
		}
	}

	void SerializableObject::Map::unpack(ByteArray& ba, ui32 size) {
		this->value.clear();

		for (ui32 i = 0; i < size; ++i) {
			SerializableObject key;
			SerializableObject value;
			key.unpack(ba);
			value.unpack(ba);
			this->value.emplace(key, value);
		}
	}


	SerializableObject::SerializableObject() :
		_type(ValueType::INVALID) {
		_value.uint64 = 0;
	}

	SerializableObject::SerializableObject(ValueType value) :
		_type(value) {
		switch (_type) {
		case ValueType::STRING:
			_value.uint64 = 0;
			break;
		case ValueType::ARRAY:
			_value.array = new Array();
			break;
		case ValueType::MAP:
			_value.map = new Map();
			break;
		case ValueType::BYTES:
			_value.internalBytes = new Bytes<false>();
			break;
		case ValueType::EXT_BYTES:
			_value.externalBytes = new Bytes<true>();
			break;
		default:
			_value.uint64 = 0;
			break;
		}
	}

	SerializableObject::SerializableObject(bool value) :
		_type(ValueType::BOOL) {
		_value.boolean = value;
	}

	SerializableObject::SerializableObject(i8 value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(ui8 value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(i16 value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(ui16 value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(i32 value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(ui32 value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(const i64& value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(const ui64& value) :
		_type(ValueType::INTEGET) {
		_value.uint64 = value;
	}

	SerializableObject::SerializableObject(f32 value) :
		_type(ValueType::FLOAT) {
		_value.number32 = value;
	}

	SerializableObject::SerializableObject(const f64& value) :
		_type(ValueType::DOUBLE) {
		_value.number64 = value;
	}

	SerializableObject::SerializableObject(const i8* value) :
		_type(ValueType::STRING) {
		if (auto size = strlen(value); size < SHORT_STRING_MAX_SIZE) {
			_writeShortString(value, size);
			_type = ValueType::SHORT_STRING;
		} else {
			_value.string = new std::string(value);
			_type = ValueType::STRING;
		}
	}

	SerializableObject::SerializableObject(const std::string& value) :
		_type(ValueType::STRING) {
		if (auto size = value.size(); size < SHORT_STRING_MAX_SIZE) {
			_writeShortString(value.c_str(), size);
			_type = ValueType::SHORT_STRING;
		} else {
			_value.string = new std::string(value);
			_type = ValueType::STRING;
		}
	}

	SerializableObject::SerializableObject(const std::string_view& value) :
		_type(ValueType::STRING) {
		if (auto size = value.size(); size < SHORT_STRING_MAX_SIZE) {
			_writeShortString(value.data(), size);
			_type = ValueType::SHORT_STRING;
		} else {
			_value.string = new std::string(value);
			_type = ValueType::STRING;
		}
	}

	SerializableObject::SerializableObject(const i8* value, ui32 size, bool copy) {
		if (copy) {
			_value.internalBytes = size ? new Bytes<false>(value, size) : nullptr;
			_type = ValueType::BYTES;
		} else {
			_value.externalBytes = size ? new Bytes<true>(value, size) : nullptr;
			_type = ValueType::EXT_BYTES;
		}
	}

	SerializableObject::SerializableObject(ByteArray& ba, bool copy) : SerializableObject(ba.getBytes(), ba.getLength(), copy) {
	}

	SerializableObject::SerializableObject(const SerializableObject& key, const SerializableObject& value) :
		_type(ValueType::MAP) {
		_value.map = new Map(key, value);
	}

	SerializableObject::SerializableObject(const SerializableObject& value) : SerializableObject(value, false) {
	}

	SerializableObject::SerializableObject(const SerializableObject& value, bool copy) :
		_type(value._type) {
		switch (_type) {
		case ValueType::STRING:
			_value.string = new std::string(*value._value.string);
			break;
		case ValueType::ARRAY:
		{
			auto arr = value._value.array;
			_value.array = copy ? arr->copy() : arr->ref<Array>();

			break;
		}
		case ValueType::MAP:
		{
			auto map = value._value.map;
			_value.map = copy ? map->copy() : map->ref<Map>();

			break;
		}
		case ValueType::BYTES:
		{
			auto bytes = value._value.internalBytes;
			_value.internalBytes = copy ? bytes->copy() : bytes->ref<Bytes<false>>();

			break;
		}
		case ValueType::EXT_BYTES:
		{
			auto bytes = value._value.externalBytes;
			_value.externalBytes = copy ? bytes->copy() : bytes->ref<Bytes<true>>();

			break;
		}
		default:
			_value.uint64 = value._value.uint64;
			break;
		}
	}

	SerializableObject& SerializableObject::operator=(const SerializableObject& value) {
		_type = value._type;
		_value.uint64 = value._value.uint64;
		switch (_type) {
		case ValueType::STRING:
			_value.string = new std::string(*value._value.string);
			break;
		case ValueType::ARRAY:
			_value.array->ref();
			break;
		case ValueType::MAP:
			_value.map->ref();
			break;
		case ValueType::BYTES:
			_value.internalBytes->ref();
			break;
		case ValueType::EXT_BYTES:
			_value.externalBytes->ref();
			break;
		default:
			break;
		}

		return *this;
	}

	SerializableObject& SerializableObject::operator=(SerializableObject&& value) {
		_type = value._type;
		_value.uint64 = value._value.uint64;
		value._value.uint64 = 0;
		value._type = ValueType::INVALID;

		return *this;
	}

	SerializableObject::SerializableObject(SerializableObject&& value) :
		_type(value._type) {
		_value.uint64 = value._value.uint64;
		value._value.uint64 = 0;
		value._type = ValueType::INVALID;
	}

	SerializableObject::~SerializableObject() {
		_freeValue();
	}

	bool SerializableObject::isContentEqual(const SerializableObject& sv) const {
		switch (_type) {
		case ValueType::INVALID:
			return sv._type == ValueType::INVALID;
			break;
		case ValueType::BOOL:
			return sv._type == ValueType::BOOL && _value.boolean == sv._value.boolean;
			break;
		case ValueType::INTEGET:
		{
			if (sv._type == ValueType::INTEGET) {
				return _value.uint64 == sv._value.uint64;
			} else if (sv._type == ValueType::FLOAT) {
				return _value.uint64 == sv._value.number32;
			} else if (sv._type == ValueType::DOUBLE) {
				return _value.uint64 == sv._value.number64;
			} else {
				return false;
			}

			break;
		}
		case ValueType::FLOAT:
		{
			if (sv._type == ValueType::INTEGET) {
				return _value.number32 == sv._value.uint64;
			} else if (sv._type == ValueType::FLOAT) {
				return _value.number32 == sv._value.number32;
			} else if (sv._type == ValueType::DOUBLE) {
				return _value.number32 == sv._value.number64;
			} else {
				return false;
			}

			break;
		}
		case ValueType::DOUBLE:
		{
			if (sv._type == ValueType::INTEGET) {
				return _value.number64 == sv._value.uint64;
			} else if (sv._type == ValueType::FLOAT) {
				return _value.number64 == sv._value.number32;
			} else if (sv._type == ValueType::DOUBLE) {
				return _value.number64 == sv._value.number64;
			} else {
				return false;
			}

			break;
		}
		case ValueType::STRING:
		{
			if (sv._type == ValueType::STRING) {
				return *_value.string == *sv._value.string;
			} else if (sv._type == ValueType::SHORT_STRING) {
				return _isContentEqual(*_value.string, sv._value.shortString);
			} else {
				return false;
			}

			break;
		}
		case ValueType::SHORT_STRING:
		{
			if (sv._type == ValueType::STRING) {
				return _isContentEqual(*sv._value.string, _value.shortString);
			} else if (sv._type == ValueType::SHORT_STRING) {
				return _isContentEqual(_value.shortString, sv._value.shortString);
			} else {
				return false;
			}

			break;
		}
		case ValueType::ARRAY:
			return sv._type == ValueType::ARRAY && _value.array->isContentEqual(sv._value.array);
			break;
		case ValueType::MAP:
			return sv._type == ValueType::MAP && _value.map->isContentEqual(sv._value.map);
			break;
		case ValueType::BYTES:
		{
			if (sv._type == ValueType::BYTES) {
				return _value.internalBytes->isContentEqual(*sv._value.internalBytes);
			} else if (sv._type == ValueType::EXT_BYTES) {
				return _value.internalBytes->isContentEqual(*sv._value.externalBytes);
			} else {
				return false;
			}

			break;
		}
		case ValueType::EXT_BYTES:
		{
			if (sv._type == ValueType::BYTES) {
				return _value.externalBytes->isContentEqual(*sv._value.internalBytes);
			} else if (sv._type == ValueType::EXT_BYTES) {
				return _value.externalBytes->isContentEqual(*sv._value.externalBytes);
			} else {
				return false;
			}

			break;
		}
		default:
			return false;
			break;
		}
	}

	bool SerializableObject::operator==(const SerializableObject& right) const {
		switch (_type) {
		case ValueType::INVALID:
			return right._type == ValueType::INVALID;
			break;
		case ValueType::BOOL:
			return right._type == ValueType::BOOL && _value.boolean == right._value.boolean;
			break;
		case ValueType::INTEGET:
		{
			if (right._type == ValueType::INTEGET) {
				return _value.uint64 == right._value.uint64;
			} else if (right._type == ValueType::FLOAT) {
				return _value.uint64 == right._value.number32;
			} else if (right._type == ValueType::DOUBLE) {
				return _value.uint64 == right._value.number64;
			} else {
				return false;
			}

			break;
		}
		case ValueType::FLOAT:
		{
			if (right._type == ValueType::INTEGET) {
				return _value.number32 == right._value.uint64;
			} else if (right._type == ValueType::FLOAT) {
				return _value.number32 == right._value.number32;
			} else if (right._type == ValueType::DOUBLE) {
				return _value.number32 == right._value.number64;
			} else {
				return false;
			}

			break;
		}
		case ValueType::DOUBLE:
		{
			if (right._type == ValueType::INTEGET) {
				return _value.number64 == right._value.uint64;
			} else if (right._type == ValueType::FLOAT) {
				return _value.number64 == right._value.number32;
			} else if (right._type == ValueType::DOUBLE) {
				return _value.number64 == right._value.number64;
			} else {
				return false;
			}

			break;
		}
		case ValueType::STRING:
		{
			if (right._type == ValueType::STRING) {
				return *_value.string == *right._value.string;
			} else if (right._type == ValueType::SHORT_STRING) {
				return _isContentEqual(*_value.string, right._value.shortString);
			} else {
				return false;
			}

			break;
		}
		case ValueType::SHORT_STRING:
		{
			if (right._type == ValueType::STRING) {
				return _isContentEqual(*right._value.string, _value.shortString);
			} else if (right._type == ValueType::SHORT_STRING) {
				return _isContentEqual(_value.shortString, right._value.shortString);
			} else {
				return false;
			}

			break;
		}
		case ValueType::ARRAY:
			return right._type == ValueType::ARRAY && _value.array == right._value.array;
			break;
		case ValueType::MAP:
			return right._type == ValueType::MAP && _value.map == right._value.map;
			break;
		case ValueType::BYTES:
			return right._type == ValueType::BYTES && _value.internalBytes == right._value.internalBytes;
			break;
		case ValueType::EXT_BYTES:
			return right._type == ValueType::EXT_BYTES && _value.externalBytes->getValue() == right._value.externalBytes->getValue() && _value.externalBytes->getSize() == right._value.externalBytes->getSize();
			break;
		default:
			return false;
			break;
		}
	}

	SerializableObject::ValueType SerializableObject::getType() const {
		return _type == ValueType::SHORT_STRING ? ValueType::STRING : _type;
	}

	bool SerializableObject::isValid() const {
		return _type != ValueType::INVALID;
	}

	ui32 SerializableObject::getSize() const {
		switch (_type) {
		case ValueType::ARRAY:
			return _value.array->value.size();
			break;
		case ValueType::MAP:
			return _value.map->value.size();
			break;
		case ValueType::BYTES:
			return _value.internalBytes->getSize();
			break;
		case ValueType::EXT_BYTES:
			return _value.externalBytes->getSize();
			break;
		case ValueType::STRING:
			return _value.string->size();
			break;
		case ValueType::SHORT_STRING:
			return strlen(_value.shortString);
			break;
		default:
			return 0;
			break;
		}
	}

	void SerializableObject::clearValues() {
		switch (_type) {
		case ValueType::ARRAY:
			_value.array->value.clear();
			break;
		case ValueType::MAP:
			_value.map->value.clear();
			break;
		case ValueType::BYTES:
			_value.internalBytes->clear();
			break;
		case ValueType::EXT_BYTES:
			_value.externalBytes->clear();
			break;
		default:
			break;
		}
	}

	bool SerializableObject::setInvalid() {
		if (_type == ValueType::INVALID) {
			return false;
		} else {
			_freeValue();
			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return true;
		}
	}

	bool SerializableObject::setArray() {
		if (_type == ValueType::ARRAY) {
			return false;
		} else {
			_freeValue();

			_value.array = new Array();
			_type = ValueType::ARRAY;

			return true;
		}
	}

	bool SerializableObject::setMap() {
		if (_type == ValueType::MAP) {
			return false;
		} else {
			_freeValue();

			_value.map = new Map();
			_type = ValueType::MAP;

			return true;
		}
	}

	bool SerializableObject::toBool(bool defaultValue) const {
		switch (_type) {
		case ValueType::BOOL:
			return _value.boolean;
			break;
		case ValueType::INTEGET:
			return _value.uint64 != 0;
			break;
		case ValueType::FLOAT:
			return _value.number32 != 0.0f;
			break;
		case ValueType::DOUBLE:
			return _value.number64 != 0.0;
			break;
		default:
			return defaultValue;
			break;
		}
	}

	std::string SerializableObject::toString(const std::string& defaultValue) const {
		switch (_type) {
		case ValueType::BOOL:
			return _value.boolean ? "true" : "false";
			break;
		case ValueType::INTEGET:
			return String::toString<i64>(_value.uint64);
			break;
		case ValueType::FLOAT:
			return String::toString<f32>(_value.number32);
			break;
		case ValueType::DOUBLE:
			return String::toString<f64>(_value.number64);
			break;
		case ValueType::STRING:
			return *_value.string;
			break;
		case ValueType::SHORT_STRING:
			return _value.shortString;
			break;
		default:
			return defaultValue;
			break;
		}
	}

	const i8* SerializableObject::toBytes() const {
		if (_type == ValueType::BYTES) {
			return _value.internalBytes->getValue();
		} else if (_type == ValueType::EXT_BYTES) {
			return _value.externalBytes->getValue();
		} else {
			return nullptr;
		}
	}

	void SerializableObject::set(bool value) {
		_setInteger(value);
	}

	void SerializableObject::set(char value) {
		_setInteger(value);
	}

	void SerializableObject::set(ui8 value) {
		_setInteger(value);
	}

	void SerializableObject::set(short value) {
		_setInteger(value);
	}

	void SerializableObject::set(ui16 value) {
		_setInteger(value);
	}

	void SerializableObject::set(int value) {
		_setInteger(value);
	}

	void SerializableObject::set(ui32 value) {
		_setInteger(value);
	}

	void SerializableObject::set(const i64& value) {
		_setInteger(value);
	}

	void SerializableObject::set(const ui64& value) {
		_setInteger(value);
	}

	void SerializableObject::set(f32 value) {
		if (_type != ValueType::FLOAT) {
			_freeValue();
			_type = ValueType::FLOAT;
		}
		_value.number32 = value;
	}

	void SerializableObject::set(const f64& value) {
		if (_type != ValueType::DOUBLE) {
			_freeValue();
			_type = ValueType::DOUBLE;
		}
		_value.number64 = value;
	}

	void SerializableObject::set(const i8* value) {
		if (_type == ValueType::STRING) {
			*_value.string = value;
		} else if (_type == ValueType::SHORT_STRING) {
			ui32 size = strlen(value);
			if (size < SHORT_STRING_MAX_SIZE) {
				_writeShortString(value, size);
			} else {
				_value.string = new std::string(value);
				_type = ValueType::STRING;
			}
		} else {
			_freeValue();

			ui32 size = strlen(value);
			if (size < SHORT_STRING_MAX_SIZE) {
				_writeShortString(value, size);
				_type = ValueType::SHORT_STRING;
			} else {
				_value.string = new std::string(value);
				_type = ValueType::STRING;
			}
		}
	}

	void SerializableObject::set(const std::string& value) {
		if (_type == ValueType::STRING) {
			*_value.string = value;
		} else if (_type == ValueType::SHORT_STRING) {
			ui32 size = value.size();
			if (size < SHORT_STRING_MAX_SIZE) {
				_writeShortString(value.c_str(), size);
			} else {
				_value.string = new std::string(value);
				_type = ValueType::STRING;
			}
		} else {
			_freeValue();

			ui32 size = value.size();
			if (size < SHORT_STRING_MAX_SIZE) {
				_writeShortString(value.c_str(), size);
				_type = ValueType::SHORT_STRING;
			} else {
				_value.string = new std::string(value);
				_type = ValueType::STRING;
			}
		}
	}

	void SerializableObject::set(const std::string_view& value) {
		if (_type == ValueType::STRING) {
			*_value.string = value;
		} else if (_type == ValueType::SHORT_STRING) {
			ui32 size = value.size();
			if (size < SHORT_STRING_MAX_SIZE) {
				_writeShortString(value.data(), size);
			} else {
				_value.string = new std::string(value);
				_type = ValueType::STRING;
			}
		} else {
			_freeValue();

			ui32 size = value.size();
			if (size < SHORT_STRING_MAX_SIZE) {
				_writeShortString(value.data(), size);
				_type = ValueType::SHORT_STRING;
			} else {
				_value.string = new std::string(value);
				_type = ValueType::STRING;
			}
		}
	}

	void SerializableObject::set(const i8* value, ui32 size, bool copy) {
		if (copy) {
			setBytes<false>();
			_value.internalBytes->setValue(value, size);
		} else {
			setBytes<true>();
			_value.externalBytes->setValue(value, size);
		}
	}

	void SerializableObject::set(const SerializableObject& value, bool copy) {
		switch (value._type) {
		case ValueType::STRING:
			set(*value._value.string);
			break;
		case ValueType::SHORT_STRING:
			set(value._value.shortString);
			break;
		case ValueType::ARRAY:
		{
			_freeValue();
			_type = ValueType::ARRAY;

			auto arr = value._value.array;
			_value.array = copy ? arr->copy() : arr->ref<Array>();

			break;
		}
		case ValueType::MAP:
		{
			_freeValue();
			_type = ValueType::MAP;

			auto map = value._value.map;
			_value.map = copy ? map->copy() : map->ref<Map>();

			break;
		}
		case ValueType::BYTES:
		{
			_freeValue();
			_type = ValueType::BYTES;

			auto bytes = value._value.internalBytes;
			_value.internalBytes = copy ? bytes->copy() : bytes->ref<Bytes<false>>();

			break;
		}
		case ValueType::EXT_BYTES:
		{
			_freeValue();
			_type = ValueType::EXT_BYTES;

			auto bytes = value._value.externalBytes;
			_value.externalBytes = copy ? bytes->copy() : bytes->ref<Bytes<true>>();

			break;
		}
		default:
		{
			_value.uint64 = value._value.uint64;
			_type = value._type;

			break;
		}
		}
	}

	SerializableObject& SerializableObject::at(ui32 index) {
		Array* arr = _getArray();
		if (index >= arr->value.size()) arr->value.resize(index + 1);
		return arr->value[index];
	}

	SerializableObject SerializableObject::tryAt(ui32 index) const {
		if (_type == ValueType::ARRAY) {
			Array* arr = _value.array;
			return index < arr->value.size() ? arr->value[index] : std::move(SerializableObject());
		} else {
			return std::move(SerializableObject());
		}
	}

	void SerializableObject::push(const SerializableObject& value) {
		_getArray()->value.emplace_back(value);
	}

	SerializableObject SerializableObject::removeAt(ui32 index) {
		if (_type == ValueType::ARRAY) {
			Array* arr = _value.array;

			if (index < arr->value.size()) {
				SerializableObject v = arr->value[index];
				arr->value.erase(arr->value.begin() + index);
				return std::move(v);
			} else {
				return std::move(SerializableObject());
			}
		} else {
			return std::move(SerializableObject());
		}
	}

	void SerializableObject::insertAt(ui32 index, const SerializableObject& value) {
		Array* arr = _getArray();
		if (index < arr->value.size()) {
			arr->value.emplace(arr->value.begin() + index, value);
		} else {
			at(index) = value;
		}
	}

	SerializableObject& SerializableObject::get(const SerializableObject& key) {
		Map* map = _getMap();

		auto itr = map->value.find(key);
		return itr == map->value.end() ? map->value.emplace(key, SerializableObject()).first->second : itr->second;
	}

	SerializableObject SerializableObject::tryGet(const SerializableObject& key) const {
		if (_type == ValueType::MAP) {
			Map* map = _value.map;

			auto itr = map->value.find(key);
			return itr == map->value.end() ? std::move(SerializableObject()) : itr->second;
		} else {
			return std::move(SerializableObject());
		}
	}

	SerializableObject& SerializableObject::insert(const SerializableObject& key, const SerializableObject& value) {
		Map* map = _getMap();

		auto itr = map->value.find(key);
		if (itr == map->value.end()) {
			return map->value.emplace(key, value).first->second;
		} else {
			itr->second = value;
			return itr->second;
		}
	}

	bool SerializableObject::has(const SerializableObject& key) const {
		return _type == ValueType::MAP ? _value.map->value.find(key) != _value.map->value.end() : false;
	}

	SerializableObject SerializableObject::remove(const SerializableObject& key) {
		if (_type == ValueType::MAP) {
			Map* map = _value.map;

			auto itr = map->value.find(key);
			if (itr == map->value.end()) {
				return std::move(SerializableObject());
			} else {
				SerializableObject v = itr->second;
				map->value.erase(itr);
				return std::move(v);
			}
		} else {
			return std::move(SerializableObject());
		}
	}

	void SerializableObject::forEach(const std::function<ForEachOperation(const SerializableObject& key, SerializableObject& value)>& callback) {
		if (_type == ValueType::ARRAY) {
			Array* arr = _value.array;
			if (arr != nullptr) {
				SerializableObject idx;
				for (ui32 i = 0, n = arr->value.size(); i < n; ++i) {
					idx.set(i);
					auto op = callback(idx, arr->value[i]);
					if ((op & ForEachOperation::ERASE) == ForEachOperation::ERASE) {
						arr->value.erase(arr->value.begin() + i);
						--i;
						--n;
					}
					if ((op & ForEachOperation::BREAK) == ForEachOperation::BREAK) break;
				}
			}
		} else if (_type == ValueType::MAP) {
			Map* map = _value.map;
			if (map != nullptr) {
				for (auto itr = map->value.begin(); itr != map->value.end();) {
					auto op = callback(itr->first, itr->second);
					if ((op & ForEachOperation::ERASE) == ForEachOperation::ERASE) {
						itr = map->value.erase(itr);
					} else {
						++itr;
					}
					if ((op & ForEachOperation::BREAK) == ForEachOperation::BREAK) break;
				}
			}
		}
	}

	void SerializableObject::pack(ByteArray& ba) const {
		switch (_type) {
		case ValueType::INVALID:
			ba.writeUInt8((ui8)_type);
			break;
		case ValueType::BOOL:
			ba.writeUInt8((ui8)(_value.boolean ? InternalValueType::BOOL_TRUE : InternalValueType::BOOL_FALSE));
			break;
		case ValueType::INTEGET:
		{
			if (_value.uint64 > INT64_MAX) {
				i64 v = _value.uint64;
				if (v == -1) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_1);
				} else if (v >= -0xFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_8BITS);
					ba.writeUInt8(-v);
				} else if (v >= -0xFFFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_16BITS);
					ba.writeUInt16(-v);
				} else if (v >= -0xFFFFFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_24BITS);
					ba.writeUInt(3, -v);
				} else if (v >= -(i64)0xFFFFFFFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_32BITS);
					ba.writeUInt32(-v);
				} else if (v >= -0xFFFFFFFFFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_40BITS);
					ba.writeUInt(5, -v);
				} else if (v >= -0xFFFFFFFFFFFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_48BITS);
					ba.writeUInt(6, -v);
				} else if (v >= -0xFFFFFFFFFFFFFFLL) {
					ba.writeUInt8((ui8)InternalValueType::N_INT_56BITS);
					ba.writeUInt(7, -v);
				} else {
					ba.writeUInt8((ui8)InternalValueType::N_INT_64BITS);
					ba.writeUInt64(-v);
				}
			} else {
				if (_value.uint64 == 0) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_0);
				} else if (_value.uint64 == 1) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_1);
				} else if (_value.uint64 <= 0xFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_8BITS);
					ba.writeUInt8(_value.uint64);
				} else if (_value.uint64 <= 0xFFFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_16BITS);
					ba.writeUInt16(_value.uint64);
				} else if (_value.uint64 <= 0xFFFFFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_24BITS);
					ba.writeUInt(3, _value.uint64);
				} else if (_value.uint64 <= 0xFFFFFFFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_32BITS);
					ba.writeUInt32(_value.uint64);
				} else if (_value.uint64 <= 0xFFFFFFFFFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_40BITS);
					ba.writeUInt(5, _value.uint64);
				} else if (_value.uint64 <= 0xFFFFFFFFFFFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_48BITS);
					ba.writeUInt(6, _value.uint64);
				} else if (_value.uint64 <= 0xFFFFFFFFFFFFFFULL) {
					ba.writeUInt8((ui8)InternalValueType::U_INT_56BITS);
					ba.writeUInt(7, _value.uint64);
				} else {
					ba.writeUInt8((ui8)InternalValueType::U_INT_64BITS);
					ba.writeUInt64(_value.uint64);
				}
			}

			break;
		}
		case ValueType::FLOAT:
		{

			f32 f = _value.number32;
			if (f == 0.0f) {
				ba.writeUInt8((ui8)InternalValueType::FLOAT_0);
			} else if (f == 0.5f) {
				ba.writeUInt8((ui8)InternalValueType::FLOAT_0_5);
			} else if (f == 1.0f) {
				ba.writeUInt8((ui8)InternalValueType::FLOAT_1);
			} else {
				i32 intValue = f;
				if (intValue == f) {
					if (intValue < 0) {
						if (intValue >= -0xFF) {
							ba.writeUInt8((ui8)InternalValueType::N_FLOAT_INT_8BITS);
							ba.writeUInt8(-intValue);
						} else if (intValue >= -0xFFFF) {
							ba.writeUInt8((ui8)InternalValueType::N_FLOAT_INT_16BITS);
							ba.writeUInt16(-intValue);
						} else if (intValue >= -0xFFFFFF) {
							ba.writeUInt8((ui8)InternalValueType::N_FLOAT_INT_24BITS);
							ba.writeUInt(3, -intValue);
						} else {
							ba.writeUInt8((ui8)_type);
							ba.writeFloat32(f);
						}
					} else {
						if (intValue <= 0xFF) {
							ba.writeUInt8((ui8)InternalValueType::U_FLOAT_INT_8BITS);
							ba.writeUInt8(intValue);
						} else if (intValue <= 0xFFFF) {
							ba.writeUInt8((ui8)InternalValueType::U_FLOAT_INT_16BITS);
							ba.writeUInt16(intValue);
						} else if (intValue <= 0xFFFFFF) {
							ba.writeUInt8((ui8)InternalValueType::U_FLOAT_INT_24BITS);
							ba.writeUInt(3, intValue);
						} else {
							ba.writeUInt8((ui8)_type);
							ba.writeFloat32(f);
						}
					}
				} else {
					ba.writeUInt8((ui8)_type);
					ba.writeFloat32(f);
				}
			}

			break;
		}
		case ValueType::DOUBLE:
		{
			f64 d = _value.number64;
			if (d == 0.0) {
				ba.writeUInt8((ui8)InternalValueType::DOUBLE_0);
			} else if (d == 0.5) {
				ba.writeUInt8((ui8)InternalValueType::DOUBLE_0_5);
			} else if (d == 1.0) {
				ba.writeUInt8((ui8)InternalValueType::DOUBLE_1);
			} else {
				ba.writeUInt8((ui8)_type);
				ba.writeFloat64(d);
			}

			break;
		}
		case ValueType::STRING:
		{
			const std::string& s = *_value.string;
			if (s.empty()) {
				ba.writeUInt8((ui8)InternalValueType::STRING_EMPTY);
			} else {
				ba.writeUInt8((ui8)_type);
				ba.writeString(s);
			}

			break;
		}
		case ValueType::SHORT_STRING:
		{
			ui32 size = strlen(_value.shortString);
			if (size == 0) {
				ba.writeUInt8((ui8)InternalValueType::STRING_EMPTY);
			} else {
				ba.writeUInt8((ui8)_type);
				ba.writeString(_value.shortString, size);
			}

			break;
		}
		case ValueType::ARRAY:
		{
			Array* arr = _value.array;
			ui32 size = arr->value.size();
			if (size == 0) {
				ba.writeUInt8((ui8)InternalValueType::ARRAY_0);
				break;
			} else if (size <= 0xFF) {
				ba.writeUInt8((ui8)InternalValueType::ARRAY_8BITS);
				ba.writeUInt8(size);
			} else if (size <= 0xFFFF) {
				ba.writeUInt8((ui8)InternalValueType::ARRAY_16BITS);
				ba.writeUInt16(size);
			} else if (size <= 0xFFFFFF) {
				ba.writeUInt8((ui8)InternalValueType::ARRAY_24BITS);
				ba.writeUInt(3, size);
			} else {
				ba.writeUInt8((ui8)InternalValueType::ARRAY_32BITS);
				ba.writeUInt32(size);
			}

			for (auto& i : arr->value) i.pack(ba);

			break;
		}
		case ValueType::MAP:
		{
			Map* map = _value.map;
			ui32 size = map->value.size();
			if (size == 0) {
				ba.writeUInt8((ui8)InternalValueType::MAP_0);
				break;
			} else if (size <= 0xFF) {
				ba.writeUInt8((ui8)InternalValueType::MAP_8BITS);
				ba.writeUInt8(size);
			} else if (size <= 0xFFFF) {
				ba.writeUInt8((ui8)InternalValueType::MAP_16BITS);
				ba.writeUInt16(size);
			} else if (size <= 0xFFFFFF) {
				ba.writeUInt8((ui8)InternalValueType::MAP_24BITS);
				ba.writeUInt(3, size);
			} else {
				ba.writeUInt8((ui8)InternalValueType::MAP_32BITS);
				ba.writeUInt32(size);
			}

			for (auto& i : map->value) {
				i.first.pack(ba);
				i.second.pack(ba);
			}

			break;
		}
		case ValueType::BYTES:
		{
			ui32 size = _value.internalBytes->getSize();
			if (size == 0) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_0);
				break;
			} else if (size <= 0xFF) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_8BITS);
				ba.writeUInt8(size);
			} else if (size <= 0xFFFF) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_16BITS);
				ba.writeUInt16(size);
			} else if (size <= 0xFFFFFF) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_24BITS);
				ba.writeUInt(3, size);
			} else {
				ba.writeUInt8((ui8)InternalValueType::BYTES_32BITS);
				ba.writeUInt32(size);
			}

			ba.writeBytes(_value.internalBytes->getValue(), 0, size);

			break;
		}
		case ValueType::EXT_BYTES:
		{
			ui32 size = _value.externalBytes->getSize();
			if (size == 0) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_0);
				break;
			} else if (size <= 0xFF) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_8BITS);
				ba.writeUInt8(size);
			} else if (size <= 0xFFFF) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_16BITS);
				ba.writeUInt16(size);
			} else if (size <= 0xFFFFFF) {
				ba.writeUInt8((ui8)InternalValueType::BYTES_24BITS);
				ba.writeUInt(3, size);
			} else {
				ba.writeUInt8((ui8)InternalValueType::BYTES_32BITS);
				ba.writeUInt32(size);
			}

			ba.writeBytes(_value.externalBytes->getValue(), 0, size);

			break;
		}
		default:
			break;
		}
	}

	void SerializableObject::unpack(ByteArray& ba) {
		if (ba.getBytesAvailable() > 0) {
			InternalValueType type = (InternalValueType)ba.readUInt8();
			switch (type) {
			case InternalValueType::UNVALID:
				setInvalid();
				break;
			case InternalValueType::FLOAT_0:
				set(0.0f);
				break;
			case InternalValueType::FLOAT_0_5:
				set(0.5f);
				break;
			case InternalValueType::FLOAT_1:
				set(1.0f);
				break;
			case InternalValueType::N_FLOAT_INT_8BITS:
				set(-(f32)ba.readUInt8());
				break;
			case InternalValueType::N_FLOAT_INT_16BITS:
				set(-(f32)ba.readUInt16());
				break;
			case InternalValueType::N_FLOAT_INT_24BITS:
				set(-(f32)ba.readUInt(3));
				break;
			case InternalValueType::U_FLOAT_INT_8BITS:
				set((f32)ba.readUInt8());
				break;
			case InternalValueType::U_FLOAT_INT_16BITS:
				set((f32)ba.readUInt16());
				break;
			case InternalValueType::U_FLOAT_INT_24BITS:
				set((f32)ba.readUInt(3));
				break;
			case InternalValueType::FLOAT:
				set(ba.readFloat32());
				break;
			case InternalValueType::DOUBLE_0:
				set(0.0);
				break;
			case InternalValueType::DOUBLE_0_5:
				set(0.5);
				break;
			case InternalValueType::DOUBLE_1:
				set(1.0);
				break;
			case InternalValueType::DOUBLE:
				set(ba.readFloat64());
				break;
			case InternalValueType::STRING_EMPTY:
				set("");
				break;
			case InternalValueType::STRING:
			case InternalValueType::SHORT_STRING:
				set(ba.readStringView());
				break;
			case InternalValueType::BOOL_TRUE:
				set(true);
				break;
			case InternalValueType::BOOL_FALSE:
				set(false);
				break;
			case InternalValueType::N_INT_1:
				set(-1);
				break;
			case InternalValueType::N_INT_8BITS:
				set(-ba.readUInt8());
				break;
			case InternalValueType::N_INT_16BITS:
				set(-ba.readUInt16());
				break;
			case InternalValueType::N_INT_24BITS:
				set(-(i64)ba.readUInt(3));
				break;
			case InternalValueType::N_INT_32BITS:
				set(-(i64)ba.readUInt32());
				break;
			case InternalValueType::N_INT_40BITS:
				set(-(i64)ba.readUInt(5));
				break;
			case InternalValueType::N_INT_48BITS:
				set(-(i64)ba.readUInt(6));
				break;
			case InternalValueType::N_INT_56BITS:
				set(-(i64)ba.readUInt(7));
				break;
			case InternalValueType::N_INT_64BITS:
				set(-(i64)ba.readUInt64());
				break;
			case InternalValueType::U_INT_0:
				set(0);
				break;
			case InternalValueType::U_INT_1:
				set(1);
				break;
			case InternalValueType::U_INT_8BITS:
				set(ba.readUInt8());
				break;
			case InternalValueType::U_INT_16BITS:
				set(ba.readUInt16());
				break;
			case InternalValueType::U_INT_24BITS:
				set(ba.readUInt(3));
				break;
			case InternalValueType::U_INT_32BITS:
				set(ba.readUInt32());
				break;
			case InternalValueType::U_INT_40BITS:
				set(ba.readUInt(5));
				break;
			case InternalValueType::U_INT_48BITS:
				set(ba.readUInt(6));
				break;
			case InternalValueType::U_INT_56BITS:
				set(ba.readUInt(7));
				break;
			case InternalValueType::U_INT_64BITS:
				set(ba.readUInt64());
				break;
			case InternalValueType::ARRAY_0:
				_unpackArray(ba, 0);
				break;
			case InternalValueType::ARRAY_8BITS:
				_unpackArray(ba, ba.readUInt8());
				break;
			case InternalValueType::ARRAY_16BITS:
				_unpackArray(ba, ba.readUInt16());
				break;
			case InternalValueType::ARRAY_24BITS:
				_unpackArray(ba, ba.readUInt(3));
				break;
			case InternalValueType::ARRAY_32BITS:
				_unpackArray(ba, ba.readUInt32());
				break;
			case InternalValueType::MAP_0:
				_unpackMap(ba, 0);
				break;
			case InternalValueType::MAP_8BITS:
				_unpackMap(ba, ba.readUInt8());
				break;
			case InternalValueType::MAP_16BITS:
				_unpackMap(ba, ba.readUInt16());
				break;
			case InternalValueType::MAP_24BITS:
				_unpackMap(ba, ba.readUInt(3));
				break;
			case InternalValueType::MAP_32BITS:
				_unpackMap(ba, ba.readUInt32());
				break;
			case InternalValueType::BYTES_0:
				_unpackBytes(ba, 0);
				break;
			case InternalValueType::BYTES_8BITS:
				_unpackBytes(ba, ba.readUInt8());
				break;
			case InternalValueType::BYTES_16BITS:
				_unpackBytes(ba, ba.readUInt16());
				break;
			case InternalValueType::BYTES_24BITS:
				_unpackBytes(ba, ba.readUInt(3));
				break;
			case InternalValueType::BYTES_32BITS:
				_unpackBytes(ba, ba.readUInt32());
				break;
			default:
				break;
			}
		} else {
			setInvalid();
		}
	}

	void SerializableObject::_unpackArray(ByteArray& ba, ui32 size) {
		auto arr = _getArray()->value;
		arr.resize(size);
		for (ui32 i = 0; i < size; ++i) arr[i].unpack(ba);
	}

	void SerializableObject::_unpackMap(ByteArray& ba, ui32 size) {
		_getMap()->unpack(ba, size);
	}

	void SerializableObject::_unpackBytes(ByteArray& ba, ui32 size) {
		_getInternalBytes()->setValue(ba.getBytes() + ba.getPosition(), size);
		ba.setPosition(ba.getPosition() + size);
	}

	bool SerializableObject::_freeValue() {
		switch (_type) {
		case ValueType::STRING:
		{
			delete _value.string;

			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return true;
		}
		case ValueType::ARRAY:
		{
			_value.array->unref();

			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return true;
		}
		case ValueType::MAP:
		{
			_value.map->unref();

			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return true;
		}
		case ValueType::BYTES:
		{
			_value.internalBytes->unref();

			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return true;
		}
		case ValueType::EXT_BYTES:
		{
			_value.externalBytes->unref();

			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return true;
		}
		default:
		{
			_value.uint64 = 0;
			_type = ValueType::INVALID;

			return false;
		}
		}
	}
}