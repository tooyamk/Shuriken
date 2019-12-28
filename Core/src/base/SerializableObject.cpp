#include "SerializableObject.h"

namespace aurora {
	SerializableObject::Array::Array() {
		ref();
	}

	SerializableObject::Array* SerializableObject::Array::copy() const {
		Array* arr = new Array();
		arr->value.resize(this->value.size());
		uint32_t idx = 0;
		for (auto& e : this->value) arr->value[idx++].set(e, true);
		return arr;
	}

	bool SerializableObject::Array::isContentEqual(Array* data) const {
		auto size = this->value.size();
		if (size == data->value.size()) {
			for (uint32_t i = 0; i < size; ++i) {
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

	void SerializableObject::Map::unpack(ByteArray& ba, uint32_t size) {
		this->value.clear();

		for (uint32_t i = 0; i < size; ++i) {
			SerializableObject key;
			SerializableObject value;
			key.unpack(ba);
			value.unpack(ba);
			this->value.emplace(key, value);
		}
	}


	SerializableObject::SerializableObject() :
		_type(Type::INVALID) {
	}

	SerializableObject::SerializableObject(Type value) :
		_type(value) {
		switch (_type) {
		case Type::STRING:
			_value[0] = 0;
			break;
		case Type::ARRAY:
			_getValue<Array*>() = new Array();
			break;
		case Type::MAP:
			_getValue<Map*>() = new Map();
			break;
		case Type::BYTES:
			_getValue<Bytes<false>*>() = new Bytes<false>();
			break;
		case Type::EXT_BYTES:
			_getValue<Bytes<true>*>() = new Bytes<true>();
			break;
		default:
			_getValue<uint64_t>() = 0;
			break;
		}
	}

	SerializableObject::SerializableObject(bool value) :
		_type(Type::BOOL) {
		_getValue<bool>() = value;
	}

	SerializableObject::SerializableObject(int8_t value) :
		_type(Type::INT) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint8_t value) :
		_type(Type::UINT) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(int16_t value) :
		_type(Type::INT) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint16_t value) :
		_type(Type::UINT) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(int32_t value) :
		_type(Type::INT) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint32_t value) :
		_type(Type::UINT) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(const int64_t& value) :
		_type(Type::INT) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(const uint64_t& value) :
		_type(Type::UINT) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(f32 value) :
		_type(Type::FLOAT) {
		_getValue<f32>() = value;
	}

	SerializableObject::SerializableObject(const f64& value) :
		_type(Type::DOUBLE) {
		_getValue<f64>() = value;
	}

	SerializableObject::SerializableObject(const char* value) :
		_type(Type::STRING) {
		if (auto size = strlen(value); size < VALUE_SIZE) {
			_writeShortString(value, size);
			_type = Type::SHORT_STRING;
		} else {
			_getValue<std::string*>() = new std::string(value);
			_type = Type::STRING;
		}
	}

	SerializableObject::SerializableObject(const std::string& value) :
		_type(Type::STRING) {
		if (auto size = value.size(); size < VALUE_SIZE) {
			_writeShortString(value.c_str(), size);
			_type = Type::SHORT_STRING;
		} else {
			_getValue<std::string*>() = new std::string(value);
			_type = Type::STRING;
		}
	}

	SerializableObject::SerializableObject(const std::string_view& value) :
		_type(Type::STRING) {
		if (auto size = value.size(); size < VALUE_SIZE) {
			_writeShortString(value.data(), size);
			_type = Type::SHORT_STRING;
		} else {
			_getValue<std::string*>() = new std::string(value);
			_type = Type::STRING;
		}
	}

	SerializableObject::SerializableObject(const uint8_t* value, size_t size, bool copy) {
		if (copy) {
			_getValue<Bytes<false>*>() = new Bytes<false>(value, size);
			_type = Type::BYTES;
		} else {
			_getValue<Bytes<true>*>() = new Bytes<true>(value, size);
			_type = Type::EXT_BYTES;
		}
	}

	SerializableObject::SerializableObject(ByteArray& ba, bool copy) : SerializableObject(ba.getSource(), ba.getLength(), copy) {
	}

	SerializableObject::SerializableObject(const SerializableObject& key, const SerializableObject& value) :
		_type(Type::MAP) {
		_getValue<Map*>() = new Map(key, value);
	}

	SerializableObject::SerializableObject(const SerializableObject& value) : SerializableObject(value, false) {
	}

	SerializableObject::SerializableObject(const SerializableObject& value, bool copy) :
		_type(value._type) {
		switch (_type) {
		case Type::STRING:
			_getValue<std::string*>() = new std::string(*value._getValue<std::string*>());
			break;
		case Type::ARRAY:
		{
			auto arr = value._getValue<Array*>();
			_getValue<Array*>() = copy ? arr->copy() : arr->ref<Array>();

			break;
		}
		case Type::MAP:
		{
			auto map = value._getValue<Map*>();
			_getValue<Map*>() = copy ? map->copy() : map->ref<Map>();

			break;
		}
		case Type::BYTES:
		{
			auto bytes = value._getValue<Bytes<false>*>();
			_getValue<Bytes<false>*>() = copy ? bytes->copy() : bytes->ref<Bytes<false>>();

			break;
		}
		case Type::EXT_BYTES:
		{
			auto bytes = value._getValue<Bytes<true>*>();
			_getValue<Bytes<true>*>() = copy ? bytes->copy() : bytes->ref<Bytes<true>>();

			break;
		}
		default:
			memcpy(_value, value._value, VALUE_SIZE);
			break;
		}
	}

	SerializableObject::SerializableObject(SerializableObject&& value) :
		_type(value._type) {
		memcpy(_value, value._value, VALUE_SIZE);
		value._type = Type::INVALID;
	}

	SerializableObject::~SerializableObject() {
		_freeValue();
	}

	bool SerializableObject::isEqual(const SerializableObject& target) const {
		switch (_type) {
		case Type::INVALID:
			return target._type == Type::INVALID;
		case Type::BOOL:
			return target._type == Type::BOOL && _getValue<bool>() == target._getValue<bool>();
		case Type::INT:
		{
			if (target._type == Type::INT) {
				return _getValue<int64_t>() == target._getValue<int64_t>();
			} else if (target._type == Type::UINT) {
				auto v = _getValue<int64_t>();
				return v < 0 ? false : v == target._getValue<uint64_t>();
			} else {
				return false;
			}
		}
		case Type::UINT:
		{
			if (target._type == Type::UINT) {
				return _getValue<uint64_t>() == target._getValue<uint64_t>();
			} else if (target._type == Type::INT) {
				auto v = target._getValue<int64_t>();
				return v < 0 ? false : v == _getValue<uint64_t>();
			} else {
				return false;
			}
		}
		case Type::FLOAT:
			return target._type == Type::FLOAT && _getValue<f32>() == target._getValue<f32>();
		case Type::DOUBLE:
			return target._type == Type::DOUBLE && _getValue<f64>() == target._getValue<f64>();
		case Type::STRING:
		{
			if (target._type == Type::STRING) {
				return *_getValue<std::string*>() == *target._getValue<std::string*>();
			} else if (target._type == Type::SHORT_STRING) {
				return _isContentEqual(*_getValue<std::string*>(), (char*)target._value);
			} else {
				return false;
			}
		}
		case Type::SHORT_STRING:
		{
			if (target._type == Type::STRING) {
				return _isContentEqual(*target._getValue<std::string*>(), (char*)_value);
			} else if (target._type == Type::SHORT_STRING) {
				return _isContentEqual((char*)_value, (char*)target._value);
			} else {
				return false;
			}
		}
		case Type::ARRAY:
			return target._type == Type::ARRAY && _getValue<Array*>() == target._getValue<Array*>();
		case Type::MAP:
			return target._type == Type::MAP && _getValue<Map*>() == target._getValue<Map*>();
		case Type::BYTES:
			return target._type == Type::BYTES && _getValue<Bytes<false>*>() == target._getValue<Bytes<false>*>();
		case Type::EXT_BYTES:
			return target._type == Type::EXT_BYTES && _getValue<Bytes<true>*>()->getValue() == target._getValue<Bytes<true>*>()->getValue() && _getValue<Bytes<true>*>()->getSize() == target._getValue<Bytes<true>*>()->getSize();
		default:
			return false;
		}
	}

	bool SerializableObject::isContentEqual(const SerializableObject& target) const {
		switch (_type) {
		case Type::INVALID:
			return target._type == Type::INVALID;
		case Type::BOOL:
			return target._type == Type::BOOL && _getValue<bool>() == target._getValue<bool>();
		case Type::INT:
			return _isEqual<int64_t>(target);
		case Type::UINT:
			return _isEqual<uint64_t>(target);
		case Type::FLOAT:
			return _isEqual<f32>(target);
		case Type::DOUBLE:
			return _isEqual<f64>(target);
		case Type::STRING:
		{
			if (target._type == Type::STRING) {
				return *_getValue<std::string*>() == *target._getValue<std::string*>();
			} else if (target._type == Type::SHORT_STRING) {
				return _isContentEqual(*_getValue<std::string*>(), (char*)target._value);
			} else {
				return false;
			}
		}
		case Type::SHORT_STRING:
		{
			if (target._type == Type::STRING) {
				return _isContentEqual(*target._getValue<std::string*>(), (char*)_value);
			} else if (target._type == Type::SHORT_STRING) {
				return _isContentEqual((char*)_value, (char*)target._value);
			} else {
				return false;
			}
		}
		case Type::ARRAY:
			return target._type == Type::ARRAY && _getValue<Array*>()->isContentEqual(target._getValue<Array*>());
		case Type::MAP:
			return target._type == Type::MAP && _getValue<Map*>()->isContentEqual(target._getValue<Map*>());
		case Type::BYTES:
		{
			if (target._type == Type::BYTES) {
				return _getValue<Bytes<false>*>()->isContentEqual(*target._getValue<Bytes<false>*>());
			} else if (target._type == Type::EXT_BYTES) {
				return _getValue<Bytes<false>*>()->isContentEqual(*target._getValue<Bytes<true>*>());
			} else {
				return false;
			}
		}
		case Type::EXT_BYTES:
		{
			if (target._type == Type::BYTES) {
				return _getValue<Bytes<true>*>()->isContentEqual(*target._getValue<Bytes<false>*>());
			} else if (target._type == Type::EXT_BYTES) {
				return _getValue<Bytes<true>*>()->isContentEqual(*target._getValue<Bytes<true>*>());
			} else {
				return false;
			}
		}
		default:
			return false;
		}
	}

	bool SerializableObject::operator==(const SerializableObject& right) const {
		switch (_type) {
		case Type::INVALID:
			return right._type == Type::INVALID;
		case Type::BOOL:
			return right._type == Type::BOOL && _getValue<bool>() == right._getValue<bool>();
		case Type::INT:
			return _isEqual<int64_t>(right);
		case Type::UINT:
			return _isEqual<uint64_t>(right);
		case Type::FLOAT:
			return _isEqual<f32>(right);
		case Type::DOUBLE:
			return _isEqual<f64>(right);
		case Type::STRING:
		{
			if (right._type == Type::STRING) {
				return *_getValue<std::string*>() == *right._getValue<std::string*>();
			} else if (right._type == Type::SHORT_STRING) {
				return _isContentEqual(*_getValue<std::string*>(), (char*)right._value);
			} else {
				return false;
			}
		}
		case Type::SHORT_STRING:
		{
			if (right._type == Type::STRING) {
				return _isContentEqual(*right._getValue<std::string*>(), (char*)_value);
			} else if (right._type == Type::SHORT_STRING) {
				return _isContentEqual((char*)_value, (char*)right._value);
			} else {
				return false;
			}
		}
		case Type::ARRAY:
			return right._type == Type::ARRAY && _getValue<Array*>() == right._getValue<Array*>();
		case Type::MAP:
			return right._type == Type::MAP && _getValue<Map*>() == right._getValue<Map*>();
		case Type::BYTES:
			return right._type == Type::BYTES && _getValue<Bytes<false>*>() == right._getValue<Bytes<false>*>();
		case Type::EXT_BYTES:
			return right._type == Type::EXT_BYTES && _getValue<Bytes<true>*>()->getValue() == right._getValue<Bytes<true>*>()->getValue() && _getValue<Bytes<true>*>()->getSize() == right._getValue<Bytes<true>*>()->getSize();
		default:
			return false;
		}
	}

	size_t SerializableObject::getSize() const {
		switch (_type) {
		case Type::ARRAY:
			return _getValue<Array*>()->value.size();
		case Type::MAP:
			return _getValue<Map*>()->value.size();
		case Type::BYTES:
			return _getValue<Bytes<false>*>()->getSize();
		case Type::EXT_BYTES:
			return _getValue<Bytes<true>*>()->getSize();
		case Type::STRING:
			return _getValue<std::string*>()->size();
		case Type::SHORT_STRING:
			return strlen((char*)_value);
		default:
			return 0;
		}
	}

	void SerializableObject::clear() {
		switch (_type) {
		case Type::ARRAY:
			_getValue<Array*>()->value.clear();
			break;
		case Type::MAP:
			_getValue<Map*>()->value.clear();
			break;
		case Type::BYTES:
			_getValue<Bytes<false>*>()->clear();
			break;
		case Type::EXT_BYTES:
			_getValue<Bytes<true>*>()->clear();
			break;
		case Type::STRING:
			_getValue<std::string*>()->clear();
			break;
		case Type::SHORT_STRING:
			_value[0] = 0;
			break;
		default:
			_getValue<uint64_t>() = 0;
			break;
		}
	}

	bool SerializableObject::setInvalid() {
		if (_type == Type::INVALID) {
			return false;
		} else {
			_freeValue();
			_type = Type::INVALID;

			return true;
		}
	}

	bool SerializableObject::setArray() {
		if (_type == Type::ARRAY) {
			return false;
		} else {
			_freeValue();
			_getValue<Array*>() = new Array();
			_type = Type::ARRAY;

			return true;
		}
	}

	bool SerializableObject::setMap() {
		if (_type == Type::MAP) {
			return false;
		} else {
			_freeValue();
			_getValue<Map*>() = new Map();
			_type = Type::MAP;

			return true;
		}
	}

	bool SerializableObject::toBool(bool defaultValue) const {
		switch (_type) {
		case Type::BOOL:
			return _getValue<bool>();
		case Type::INT:
			return _getValue<int64_t>();
		case Type::UINT:
			return _getValue<uint64_t>();
		case Type::FLOAT:
			return _getValue<f32>() != 0.0f;
		case Type::DOUBLE:
			return _getValue<f64>() != 0.0;
		case Type::INVALID:
			return false;
		default:
			return defaultValue;
		}
	}

	std::string SerializableObject::toString(const std::string& defaultValue) const {
		switch (_type) {
		case Type::BOOL:
			return _getValue<bool>() ? "true" : "false";
		case Type::INT:
			return String::toString<int64_t>(_getValue<int64_t>());
		case Type::UINT:
			return String::toString<uint64_t>(_getValue<uint64_t>());
		case Type::FLOAT:
			return String::toString<f32>(_getValue<f32>());
		case Type::DOUBLE:
			return String::toString<f64>(_getValue<f64>());
		case Type::STRING:
			return *_getValue<std::string*>();
		case Type::SHORT_STRING:
			return (char*)_value;
		case Type::INVALID:
			return "";
		default:
			return defaultValue;
		}
	}

	const uint8_t* SerializableObject::toBytes() const {
		if (_type == Type::BYTES) {
			return _getValue<Bytes<false>*>()->getValue();
		} else if (_type == Type::EXT_BYTES) {
			return _getValue<Bytes<true>*>()->getValue();
		} else {
			return nullptr;
		}
	}

	void SerializableObject::set(const char* value) {
		if (_type == Type::STRING) {
			*_getValue<std::string*>() = value;
		} else if (_type == Type::SHORT_STRING) {
			if (auto size = strlen(value); size < VALUE_SIZE) {
				_writeShortString(value, size);
			} else {
				_getValue<std::string*>() = new std::string(value);
				_type = Type::STRING;
			}
		} else {
			_freeValue();

			if (auto size = strlen(value); size < VALUE_SIZE) {
				_writeShortString(value, size);
				_type = Type::SHORT_STRING;
			} else {
				_getValue<std::string*>() = new std::string(value);
				_type = Type::STRING;
			}
		}
	}

	void SerializableObject::set(const std::string& value) {
		if (_type == Type::STRING) {
			*_getValue<std::string*>() = value;
		} else if (_type == Type::SHORT_STRING) {
			if (auto size = value.size(); size < VALUE_SIZE) {
				_writeShortString(value.c_str(), size);
			} else {
				_getValue<std::string*>() = new std::string(value);
				_type = Type::STRING;
			}
		} else {
			_freeValue();

			if (auto size = value.size(); size < VALUE_SIZE) {
				_writeShortString(value.c_str(), size);
				_type = Type::SHORT_STRING;
			} else {
				_getValue<std::string*>() = new std::string(value);
				_type = Type::STRING;
			}
		}
	}

	void SerializableObject::set(const std::string_view& value) {
		if (_type == Type::STRING) {
			*_getValue<std::string*>() = value;
		} else if (_type == Type::SHORT_STRING) {
			if (auto size = value.size(); size < VALUE_SIZE) {
				_writeShortString(value.data(), size);
			} else {
				_getValue<std::string*>() = new std::string(value);
				_type = Type::STRING;
			}
		} else {
			_freeValue();

			if (auto size = value.size(); size < VALUE_SIZE) {
				_writeShortString(value.data(), size);
				_type = Type::SHORT_STRING;
			} else {
				_getValue<std::string*>() = new std::string(value);
				_type = Type::STRING;
			}
		}
	}

	void SerializableObject::set(const uint8_t* value, size_t size, bool copy) {
		if (copy) {
			setBytes<false>();
			_getValue<Bytes<false>*>()->setValue(value, size);
		} else {
			setBytes<true>();
			_getValue<Bytes<true>*>()->setValue(value, size);
		}
	}

	void SerializableObject::set(const SerializableObject& value, bool copy) {
		switch (value._type) {
		case Type::STRING:
			set(*value._getValue<std::string*>());
			break;
		case Type::SHORT_STRING:
			set(value._value);
			break;
		case Type::ARRAY:
		{
			_freeValue();
			_type = Type::ARRAY;

			auto arr = value._getValue<Array*>();
			_getValue<Array*>() = copy ? arr->copy() : arr->ref<Array>();

			break;
		}
		case Type::MAP:
		{
			_freeValue();
			_type = Type::MAP;

			auto map = value._getValue<Map*>();
			_getValue<Map*>() = copy ? map->copy() : map->ref<Map>();

			break;
		}
		case Type::BYTES:
		{
			_freeValue();
			_type = Type::BYTES;

			auto bytes = value._getValue<Bytes<false>*>();
			_getValue<Bytes<false>*>() = copy ? bytes->copy() : bytes->ref<Bytes<false>>();

			break;
		}
		case Type::EXT_BYTES:
		{
			_freeValue();
			_type = Type::EXT_BYTES;

			auto bytes = value._getValue<Bytes<true>*>();
			_getValue<Bytes<true>*>() = copy ? bytes->copy() : bytes->ref<Bytes<true>>();

			break;
		}
		default:
		{
			memcpy(_value, value._value, VALUE_SIZE);
			_type = value._type;

			break;
		}
		}
	}

	SerializableObject& SerializableObject::at(size_t index) {
		Array* arr = _getArray();
		if (index >= arr->value.size()) arr->value.resize(index + 1);
		return arr->value[index];
	}

	SerializableObject SerializableObject::tryAt(size_t index) const {
		if (_type == Type::ARRAY) {
			Array* arr = _getValue<Array*>();
			return index < arr->value.size() ? arr->value[index] : std::move(SerializableObject());
		} else {
			return std::move(SerializableObject());
		}
	}

	void SerializableObject::push(const SerializableObject& value) {
		_getArray()->value.emplace_back(value);
	}

	SerializableObject SerializableObject::removeAt(size_t index) {
		if (_type == Type::ARRAY) {
			Array* arr = _getValue<Array*>();

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

	void SerializableObject::insertAt(size_t index, const SerializableObject& value) {
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
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

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
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

			return map->value.find(key) != map->value.end();
		} else {
			return false;
		}
	}

	SerializableObject SerializableObject::remove(const SerializableObject& key) {
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

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

	void SerializableObject::forEach(const std::function<void(const SerializableObject& key, const SerializableObject& value)>& callback) const {
		if (_type == Type::ARRAY) {
			Array* arr = _getValue<Array*>();
			if (arr) {
				SerializableObject idx;
				for (size_t i = 0, n = arr->value.size(); i < n; ++i) {
					idx.set(i);
					callback(idx, arr->value[i]);
				}
			}
		} else if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();
			if (map) {
				for (auto itr = map->value.begin(); itr != map->value.end();) callback(itr->first, itr->second);
			}
		}
	}

	void SerializableObject::forEach(const std::function<bool(const SerializableObject& key, const SerializableObject& value)>& callback) const {
		if (_type == Type::ARRAY) {
			Array* arr = _getValue<Array*>();
			if (arr) {
				SerializableObject idx;
				for (size_t i = 0, n = arr->value.size(); i < n; ++i) {
					idx.set(i);
					if (!callback(idx, arr->value[i])) break;
				}
			}
		} else if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();
			if (map) {
				for (auto itr = map->value.begin(); itr != map->value.end();) {
					if (!callback(itr->first, itr->second)) break;
				}
			}
		}
	}

	void SerializableObject::forEach(const std::function<ForEachOperation(const SerializableObject& key, SerializableObject& value)>& callback) {
		if (_type == Type::ARRAY) {
			Array* arr = _getValue<Array*>();
			if (arr != nullptr) {
				SerializableObject idx;
				for (size_t i = 0, n = arr->value.size(); i < n; ++i) {
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
		} else if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();
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
		case Type::INVALID:
			ba.write<uint8_t>((uint8_t)_type);
			break;
		case Type::BOOL:
			ba.write<uint8_t>((uint8_t)(_getValue<bool>() ? InternalType::BOOL_TRUE : InternalType::BOOL_FALSE));
			break;
		case Type::INT:
		{
			auto v = _getValue<int64_t>();
			if (v < 0) {
				if (v == -1) {
					ba.write<uint8_t>((uint8_t)InternalType::N_INT_1);
				} else {
					_packUInt(ba, -v, (uint8_t)InternalType::N_INT_8BITS);
				}
			} else {
				if (v == 0) {
					ba.write<uint8_t>((uint8_t)InternalType::P_INT_0);
				} else if (v == 1) {
					ba.write<uint8_t>((uint8_t)InternalType::P_INT_1);
				} else {
					_packUInt(ba, v, (uint8_t)InternalType::P_INT_8BITS);
				}
			}

			break;
		}
		case Type::UINT:
		{
			auto v = _getValue<uint64_t>();
			if (v == 0) {
				ba.write<uint8_t>((uint8_t)InternalType::P_INT_0);
			} else if (v == 1) {
				ba.write<uint8_t>((uint8_t)InternalType::P_INT_1);
			} else {
				_packUInt(ba, v, (uint8_t)InternalType::P_INT_8BITS);
			}

			break;
		}
		case Type::FLOAT:
		{

			f32 f = _getValue<f32>();
			if (f == 0.0f) {
				ba.write<uint8_t>((uint8_t)InternalType::FLT_0);
			} else if (f == 0.5f) {
				ba.write<uint8_t>((uint8_t)InternalType::FLT_0_5);
			} else if (f == 1.0f) {
				ba.write<uint8_t>((uint8_t)InternalType::FLT_1);
			} else {
				ba.write<uint8_t>((uint8_t)_type);
				ba.write<f32>(f);
			}

			break;
		}
		case Type::DOUBLE:
		{
			f64 d = _getValue<f64>();
			if (d == 0.0) {
				ba.write<uint8_t>((uint8_t)InternalType::DBL_0);
			} else if (d == 0.5) {
				ba.write<uint8_t>((uint8_t)InternalType::DBL_0_5);
			} else if (d == 1.0) {
				ba.write<uint8_t>((uint8_t)InternalType::DBL_1);
			} else {
				ba.write<uint8_t>((uint8_t)_type);
				ba.write<f64>(d);
			}

			break;
		}
		case Type::STRING:
		{
			auto& s = *_getValue<std::string*>();
			if (s.empty()) {
				ba.write<uint8_t>((uint8_t)InternalType::STRING_EMPTY);
			} else {
				ba.write<uint8_t>((uint8_t)_type);
				ba.writeString(s);
			}

			break;
		}
		case Type::SHORT_STRING:
		{
			auto size = strlen((char*)_value);
			if (size == 0) {
				ba.write<uint8_t>((uint8_t)InternalType::STRING_EMPTY);
			} else {
				ba.write<uint8_t>((uint8_t)_type);
				ba.writeString((char*)_value, size);
			}

			break;
		}
		case Type::ARRAY:
		{
			Array* arr = _getValue<Array*>();
			auto size = arr->value.size();
			if (size == 0) {
				ba.write<uint8_t>((uint8_t)InternalType::ARRAY_0);
				break;
			} else {
				_packUInt(ba, size, (uint8_t)InternalType::ARRAY_8BITS);
			}

			for (auto& i : arr->value) i.pack(ba);

			break;
		}
		case Type::MAP:
		{
			Map* map = _getValue<Map*>();
			auto size = map->value.size();
			if (size == 0) {
				ba.write<uint8_t>((uint8_t)InternalType::MAP_0);
				break;
			} else {
				_packUInt(ba, size, (uint8_t)InternalType::MAP_8BITS);
			}

			for (auto& i : map->value) {
				i.first.pack(ba);
				i.second.pack(ba);
			}

			break;
		}
		case Type::BYTES:
		{
			uint32_t size = _getValue<Bytes<false>*>()->getSize();
			if (size == 0) {
				ba.write<uint8_t>((uint8_t)InternalType::BYTES_0);
				break;
			} else if (size <= uintMax<8>()) {
				_packUInt(ba, size, (uint8_t)InternalType::BYTES_8BITS);
			}

			ba.writeBytes(_getValue<Bytes<false>*>()->getValue(), size);

			break;
		}
		case Type::EXT_BYTES:
		{
			uint32_t size = _getValue<Bytes<true>*>()->getSize();
			if (size == 0) {
				ba.write<uint8_t>((uint8_t)InternalType::BYTES_0);
				break;
			} else {
				_packUInt(ba, size, (uint8_t)InternalType::BYTES_8BITS);
			}

			ba.writeBytes(_getValue<Bytes<true>*>()->getValue(), size);

			break;
		}
		default:
			break;
		}
	}

	void SerializableObject::_packUInt(ByteArray& ba, uint64_t val, uint8_t typeBegin) const {
		if (val <= uintMax<8>()) {
			ba.write<uint8_t>(typeBegin);
			ba.write<uint8_t>(val);
		} else if (val <= uintMax<16>()) {
			ba.write<uint8_t>(typeBegin + 1);
			ba.write<uint16_t>(val);
		} else if (val <= uintMax<24>()) {
			ba.write<uint8_t>(typeBegin + 2);
			ba.writeUInt(3, val);
		} else if (val <= uintMax<32>()) {
			ba.write<uint8_t>(typeBegin + 3);
			ba.write<uint32_t>(val);
		} else if (val <= uintMax<40>()) {
			ba.write<uint8_t>(typeBegin + 4);
			ba.writeUInt(5, val);
		} else if (val <= uintMax<48>()) {
			ba.write<uint8_t>(typeBegin + 5);
			ba.writeUInt(5, val);
		} else if (val <= uintMax<56>()) {
			ba.write<uint8_t>(typeBegin + 6);
			ba.writeUInt(5, val);
		} else {
			ba.write<uint8_t>(typeBegin + 7);
			ba.write<uint64_t>(val);
		}
	}

	void SerializableObject::unpack(ByteArray& ba) {
		if (ba.getBytesAvailable() > 0) {
			InternalType type = (InternalType)ba.read<uint8_t>();
			switch (type) {
			case InternalType::UNVALID:
				setInvalid();
				break;
			case InternalType::FLT_0:
				set(0.0f);
				break;
			case InternalType::FLT_0_5:
				set(0.5f);
				break;
			case InternalType::FLT_1:
				set(1.0f);
				break;
			case InternalType::FLOAT:
				set(ba.read<f32>());
				break;
			case InternalType::DBL_0:
				set(0.0);
				break;
			case InternalType::DBL_0_5:
				set(0.5);
				break;
			case InternalType::DBL_1:
				set(1.0);
				break;
			case InternalType::DOUBLE:
				set(ba.read<f64>());
				break;
			case InternalType::STRING_EMPTY:
				set("");
				break;
			case InternalType::STRING:
			case InternalType::SHORT_STRING:
				set(ba.readStringView());
				break;
			case InternalType::BOOL_TRUE:
				set(true);
				break;
			case InternalType::BOOL_FALSE:
				set(false);
				break;
			case InternalType::N_INT_1:
				set(-1);
				break;
			case InternalType::N_INT_8BITS:
				set(-ba.read<uint8_t>());
				break;
			case InternalType::N_INT_16BITS:
				set(-ba.read<uint16_t>());
				break;
			case InternalType::N_INT_24BITS:
				set(-(int64_t)ba.readUInt(3));
				break;
			case InternalType::N_INT_32BITS:
				set(-(int64_t)ba.read<uint32_t>());
				break;
			case InternalType::N_INT_40BITS:
				set(-(int64_t)ba.readUInt(5));
				break;
			case InternalType::N_INT_48BITS:
				set(-(int64_t)ba.readUInt(6));
				break;
			case InternalType::N_INT_56BITS:
				set(-(int64_t)ba.readUInt(7));
				break;
			case InternalType::N_INT_64BITS:
				set(-(int64_t)ba.read<uint64_t>());
				break;
			case InternalType::P_INT_0:
				set(0);
				break;
			case InternalType::P_INT_1:
				set(1);
				break;
			case InternalType::P_INT_8BITS:
				set(ba.read<uint8_t>());
				break;
			case InternalType::P_INT_16BITS:
				set(ba.read<uint16_t>());
				break;
			case InternalType::P_INT_24BITS:
				set(ba.readUInt(3));
				break;
			case InternalType::P_INT_32BITS:
				set(ba.read<uint32_t>());
				break;
			case InternalType::P_INT_40BITS:
				set(ba.readUInt(5));
				break;
			case InternalType::P_INT_48BITS:
				set(ba.readUInt(6));
				break;
			case InternalType::P_INT_56BITS:
				set(ba.readUInt(7));
				break;
			case InternalType::P_INT_64BITS:
				set(ba.read<uint64_t>());
				break;
			case InternalType::ARRAY_0:
				_unpackArray(ba, 0);
				break;
			case InternalType::ARRAY_8BITS:
				_unpackArray(ba, ba.read<uint8_t>());
				break;
			case InternalType::ARRAY_16BITS:
				_unpackArray(ba, ba.read<uint16_t>());
				break;
			case InternalType::ARRAY_24BITS:
				_unpackArray(ba, ba.readUInt(3));
				break;
			case InternalType::ARRAY_32BITS:
				_unpackArray(ba, ba.read<uint32_t>());
				break;
			case InternalType::ARRAY_40BITS:
				_unpackArray(ba, ba.readUInt(5));
				break;
			case InternalType::ARRAY_48BITS:
				_unpackArray(ba, ba.readUInt(6));
				break;
			case InternalType::ARRAY_56BITS:
				_unpackArray(ba, ba.readUInt(7));
				break;
			case InternalType::ARRAY_64BITS:
				_unpackArray(ba, ba.read<uint64_t>());
				break;
			case InternalType::MAP_0:
				_unpackMap(ba, 0);
				break;
			case InternalType::MAP_8BITS:
				_unpackMap(ba, ba.read<uint8_t>());
				break;
			case InternalType::MAP_16BITS:
				_unpackMap(ba, ba.read<uint16_t>());
				break;
			case InternalType::MAP_24BITS:
				_unpackMap(ba, ba.readUInt(3));
				break;
			case InternalType::MAP_32BITS:
				_unpackMap(ba, ba.read<uint32_t>());
				break;
			case InternalType::MAP_40BITS:
				_unpackMap(ba, ba.readUInt(5));
				break;
			case InternalType::MAP_48BITS:
				_unpackMap(ba, ba.readUInt(6));
				break;
			case InternalType::MAP_56BITS:
				_unpackMap(ba, ba.readUInt(7));
				break;
			case InternalType::MAP_64BITS:
				_unpackMap(ba, ba.read<uint64_t>());
				break;
			case InternalType::BYTES_0:
				_unpackBytes(ba, 0);
				break;
			case InternalType::BYTES_8BITS:
				_unpackBytes(ba, ba.read<uint8_t>());
				break;
			case InternalType::BYTES_16BITS:
				_unpackBytes(ba, ba.read<uint16_t>());
				break;
			case InternalType::BYTES_24BITS:
				_unpackBytes(ba, ba.readUInt(3));
				break;
			case InternalType::BYTES_32BITS:
				_unpackBytes(ba, ba.read<uint32_t>());
				break;
			case InternalType::BYTES_40BITS:
				_unpackBytes(ba, ba.readUInt(5));
				break;
			case InternalType::BYTES_48BITS:
				_unpackBytes(ba, ba.readUInt(6));
				break;
			case InternalType::BYTES_56BITS:
				_unpackBytes(ba, ba.readUInt(7));
				break;
			case InternalType::BYTES_64BITS:
				_unpackBytes(ba, ba.read<uint64_t>());
				break;
			default:
				break;
			}
		} else {
			setInvalid();
		}
	}

	void SerializableObject::_unpackArray(ByteArray& ba, size_t size) {
		auto arr = _getArray()->value;
		arr.resize(size);
		for (size_t i = 0; i < size; ++i) arr[i].unpack(ba);
	}

	void SerializableObject::_unpackMap(ByteArray& ba, size_t size) {
		_getMap()->unpack(ba, size);
	}

	void SerializableObject::_unpackBytes(ByteArray& ba, size_t size) {
		_getBytes<false>()->setValue(ba.getSource() + ba.getPosition(), size);
		ba.setPosition(ba.getPosition() + size);
	}

	bool SerializableObject::_freeValue() {
		switch (_type) {
		case Type::STRING:
		{
			delete _getValue<std::string*>();
			_type = Type::INVALID;

			return true;
		}
		case Type::ARRAY:
		{
			_getValue<Array*>()->unref();
			_type = Type::INVALID;

			return true;
		}
		case Type::MAP:
		{
			_getValue<Map*>()->unref();
			_type = Type::INVALID;

			return true;
		}
		case Type::BYTES:
		{
			_getValue<Bytes<false>*>()->unref();
			_type = Type::INVALID;

			return true;
		}
		case Type::EXT_BYTES:
		{
			_getValue<Bytes<true>*>()->unref();
			_type = Type::INVALID;

			return true;
		}
		default:
			_type = Type::INVALID;
			return false;
		}
	}
}