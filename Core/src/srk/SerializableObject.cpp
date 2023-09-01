#include "SerializableObject.h"

namespace srk {
	SerializableObject::Array::Array() {
		ref();
	}

	SerializableObject::Array* SerializableObject::Array::copy(Flag flag) const {
		auto arr = new Array();
		arr->value.resize(this->value.size());
		uint32_t idx = 0;
		for (auto& e : this->value) arr->value[idx++].wrap().set(e, flag);
		return arr;
	}

	bool SerializableObject::Array::isContentEqual(Array* data) const {
		auto size = this->value.size();
		if (size == data->value.size()) {
			for (uint32_t i = 0; i < size; ++i) {
				if (!this->value[i].wrap().isContentEqual(data->value[i])) return false;
			}

			return true;
		} else {
			return false;
		}
	}


	SerializableObject::Map::Map() {
		ref();
	}

	SerializableObject::Map* SerializableObject::Map::copy(Flag flag) const {
		auto map = new Map();
		for (auto& itr : this->value) {
			SerializableObject k(itr.first, flag), v(itr.second, flag);
			map->value.emplace((SerializableObjectWrapper&)k, (SerializableObjectWrapper&)v);
		}
		return map;
	}

	bool SerializableObject::Map::isContentEqual(Map* data) const {
		auto size = this->value.size();
		if (size == data->value.size()) {
			for (auto& itr : this->value) {
				if (auto itr2 = data->value.find(itr.first); itr2 == data->value.end() || !itr.second.wrap().isContentEqual(itr2->second)) return false;
			}

			return true;
		} else {
			return false;
		}
	}

	void SerializableObject::Map::unpack(ByteArray& ba, size_t size, Flag flag) {
		value.clear();

		SerializableObject k, v;
		for (size_t i = 0; i < size; ++i) {
			k.unpack(ba, flag);
			v.unpack(ba, flag);
			value.emplace((SerializableObjectWrapper&&)std::move(k), (SerializableObjectWrapper&&)std::move(v));
		}
	}


	SerializableObject::Str::Str(const char* data, size_t size) :
		size(size) {
		ref();
		this->data = new char[size + 1];
		this->data[size] = 0;
		memcpy(this->data, data, size);
	}

	SerializableObject::Str::~Str() {
		delete[] data;
	}


	SerializableObject::Bytes::Bytes() :
		_data(nullptr),
		_size(0) {
		ref();
	}

	SerializableObject::Bytes::Bytes(const uint8_t* data, size_t size) :
		_size(size) {

		if (size) {
			_data = new uint8_t[size];
			memcpy(_data, data, size);
		} else {
			_data = nullptr;
		}

		ref();
	}

	SerializableObject::Bytes::~Bytes() {
		if (_data != nullptr) delete[] _data;
	}

	void SerializableObject::Bytes::setValue(const uint8_t* data, size_t size) {
		if (!size) {
			clear();
		} else {
			if (_data == nullptr) {
				_size = size;
				_data = new uint8_t[_size];
				if (data) memcpy(_data, data, _size);
			} else if (_size == size) {
				if (data) memcpy(_data, data, _size);
			} else {
				delete[] _data;
				_size = size;
				_data = new uint8_t[_size];
				if (data) memcpy(_data, data, size);
			}
		}
	}


	SerializableObject::SerializableObject() noexcept :
		_type(Type::INVALID),
		_flag(Flag::NONE) {
	}

	SerializableObject::SerializableObject(Type value) :
		_type(value),
		_flag(Flag::NONE) {
		switch (_type) {
		case Type::STRING:
		case Type::SHORT_STRING:
		{
			_value[0] = 0;
			_type = Type::SHORT_STRING;

			break;
		}
		case Type::STRING_VIEW:
		{
			auto& sv = _getValue<StrView>();
			sv.data = nullptr;
			sv.size = 0;

			break;
		}
		case Type::ARRAY:
			_getValue<Array*>() = new Array();
			break;
		case Type::MAP:
			_getValue<Map*>() = new Map();
			break;
		case Type::BYTES:
			_getValue<Bytes*>() = new Bytes();
			break;
		case Type::EXT_BYTES:
		{
			auto& bv = _getValue<BytesView>();
			bv.data = nullptr;
			bv.size = 0;

			break;
		}
		default:
			_getValue<uint64_t>() = 0;
			break;
		}
	}

	SerializableObject::SerializableObject(bool value) noexcept :
		_type(Type::BOOL),
		_flag(Flag::NONE) {
		_getValue<bool>() = value;
	}

	SerializableObject::SerializableObject(int8_t value) noexcept :
		_type(Type::INT),
		_flag(Flag::NONE) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint8_t value) noexcept :
		_type(Type::UINT),
		_flag(Flag::NONE) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(int16_t value) noexcept :
		_type(Type::INT),
		_flag(Flag::NONE) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint16_t value) noexcept :
		_type(Type::UINT),
		_flag(Flag::NONE) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(int32_t value) noexcept :
		_type(Type::INT),
		_flag(Flag::NONE) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint32_t value) noexcept :
		_type(Type::UINT),
		_flag(Flag::NONE) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(int64_t value) noexcept :
		_type(Type::INT),
		_flag(Flag::NONE) {
		_getValue<int64_t>() = value;
	}

	SerializableObject::SerializableObject(uint64_t value) noexcept :
		_type(Type::UINT),
		_flag(Flag::NONE) {
		_getValue<uint64_t>() = value;
	}

	SerializableObject::SerializableObject(float32_t value) noexcept :
		_type(Type::FLOAT32),
		_flag(Flag::NONE) {
		_getValue<float32_t>() = value;
	}

	SerializableObject::SerializableObject(float64_t value) noexcept :
		_type(Type::FLOAT64),
		_flag(Flag::NONE) {
		_getValue<float64_t>() = value;
	}

	SerializableObject::SerializableObject(const std::string_view& value, Flag flag) {
		using namespace srk::enum_operators;

		if ((flag & Flag::COPY) == Flag::COPY) {
			if (auto size = value.size(); size < VALUE_SIZE) {
				_writeShortString(value.data(), size);
				_type = Type::SHORT_STRING;
				_flag = Flag::NONE;
			} else {
				_getValue<Str*>() = new Str(value.data(), size);
				_type = Type::STRING;
				_flag = Flag::NONE;
			}
		} else {
			_writeStringView(value.data(), value.size());
			_type = Type::STRING_VIEW;
			_flag = flag & Flag::TO_COPY;
		}
	}

	SerializableObject::SerializableObject(const void* value, size_t size, Flag flag) {
		using namespace srk::enum_operators;

		if ((flag & Flag::COPY) == Flag::COPY) {
			if (size <= VALUE_SIZE) {
				memcpy(_value, value, size);
				if (size < VALUE_SIZE) {
					_value[VALUE_SIZE - 1] = size;
					_type = Type::SHORT_BYTES_LE_15;
				} else {
					_type = Type::SHORT_BYTES16;
				}
			} else {
				_getValue<Bytes*>() = new Bytes((const uint8_t*)value, size);
				_type = Type::BYTES;
			}
			
			_flag = Flag::NONE;
		} else {
			auto& bv = _getValue<BytesView>();
			bv.data = (const uint8_t*)value;
			bv.size = size;
			_type = Type::EXT_BYTES;
			_flag = flag & Flag::TO_COPY;
		}
	}

	SerializableObject::SerializableObject(const ByteArray& ba, Flag flag) : SerializableObject(ba.getSource(), ba.getLength(), flag) {
	}

	SerializableObject::SerializableObject(const SerializableObject& value) : SerializableObject(value, Flag::NONE) {
	}

	SerializableObject::SerializableObject(const SerializableObject& value, Flag flag) :
		_type(value._type),
		_flag(Flag::NONE) {
		using namespace srk::enum_operators;

		switch (_type) {
		case Type::STRING:
			_getValue<Str*>() = (value._getValue<Str*>())->ref<Str>();
			break;
		case Type::STRING_VIEW:
		{
			auto& sv = value._getValue<StrView>();
			if ((flag & Flag::COPY) != Flag::NONE || (value._flag & Flag::TO_COPY) != Flag::NONE) {
				if (sv.size < VALUE_SIZE) {
					_writeShortString(sv.data, sv.size);
					_type = Type::SHORT_STRING;
				} else {
					_getValue<Str*>() = new Str(sv.data, sv.size);
					_type = Type::STRING;
				}
			} else {
				_getValue<StrView>() = sv;
				_flag = flag & Flag::TO_COPY;
			}

			break;
		}
		case Type::ARRAY:
		{
			auto arr = value._getValue<Array*>();
			_getValue<Array*>() = (flag & (Flag::COPY | Flag::DETACH_COPY)) != Flag::NONE ? arr->copy(flag) : arr->ref<Array>();

			break;
		}
		case Type::MAP:
		{
			auto map = value._getValue<Map*>();
			_getValue<Map*>() = (flag & (Flag::COPY | Flag::DETACH_COPY)) != Flag::NONE ? map->copy(flag) : map->ref<Map>();

			break;
		}
		case Type::BYTES:
		{
			auto val = value._getValue<Bytes*>();
			_getValue<Bytes*>() = val->ref<Bytes>();

			break;
		}
		case Type::EXT_BYTES:
		{
			if ((flag & Flag::COPY) != Flag::NONE || (value._flag & Flag::TO_COPY) != Flag::NONE) {
				_getValue<Bytes*>() = value._getValue<Bytes*>()->copy();
				_type = Type::BYTES;
			} else {
				memcpy(_value, value._value, VALUE_SIZE);
			}

			break;
		}
		default:
			memcpy(_value, value._value, VALUE_SIZE);
			break;
		}
	}

	SerializableObject::SerializableObject(SerializableObject&& value) noexcept :
		_type(value._type),
		_flag(value._flag) {
		memcpy(_value, value._value, VALUE_SIZE);
		value._type = Type::INVALID;
		value._flag = Flag::NONE;
	}

	SerializableObject::~SerializableObject() {
		_freeValue();
	}

	bool SerializableObject::equal(const SerializableObject& target) const {
		using namespace srk::enum_operators;

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
		case Type::FLOAT32:
			return target._type == Type::FLOAT32 && _getValue<float32_t>() == target._getValue<float32_t>();
		case Type::FLOAT64:
			return target._type == Type::FLOAT64 && _getValue<float64_t>() == target._getValue<float64_t>();
		case Type::STRING:
		{
			switch (target._type) {
			case Type::STRING:
			{
				auto s1 = _getValue<Str*>();
				auto s2 = target._getValue<Str*>();
				return _isContentEqual(s1->data, s1->size, s2->data, s2->size);
			}
			case Type::SHORT_STRING:
			{
				auto s = _getValue<Str*>();
				return _isContentEqual(s->data, s->size, target._value, strlen((char*)target._value));
			}
			case Type::STRING_VIEW:
			{
				auto s = _getValue<Str*>();
				auto& sv = target._getValue<StrView>();
				return _isContentEqual(s->data, s->size, sv.data, sv.size);
			}
			default:
				return false;
			}
		}
		case Type::SHORT_STRING:
		{
			switch (target._type) {
			case Type::STRING:
			{
				auto s = target._getValue<Str*>();
				return _isContentEqual(s->data, s->size, _value, strlen((char*)_value));
			}
			case Type::SHORT_STRING:
				return _isContentEqual(_value, strlen((char*)_value), target._value, strlen((char*)target._value));
			case Type::STRING_VIEW:
			{
				auto& sv = target._getValue<StrView>();
				return _isContentEqual(_value, strlen((char*)_value), sv.data, sv.size);
			}
			default:
				return false;
			}
		}
		case Type::STRING_VIEW:
		{
			switch (target._type) {
			case Type::STRING:
			{
				auto s = target._getValue<Str*>();
				auto& sv = _getValue<StrView>();
				return _isContentEqual(s->data, s->size, sv.data, sv.size);
			}
			case Type::SHORT_STRING:
			{
				auto sv = _getValue<StrView>();
				return _isContentEqual(sv.data, sv.size, target._value, strlen((char*)target._value));
			}
			case Type::STRING_VIEW:
			{
				auto& sv1 = _getValue<StrView>();
				auto& sv2 = target._getValue<StrView>();
				return _isContentEqual(sv1.data, sv1.size, sv2.data, sv2.size);
			}
			default:
				return false;
			}
		}
		case Type::ARRAY:
			return target._type == Type::ARRAY && _getValue<Array*>() == target._getValue<Array*>();
		case Type::MAP:
			return target._type == Type::MAP && _getValue<Map*>() == target._getValue<Map*>();
		case Type::BYTES:
		{
			if (target._type == Type::BYTES) return _getValue<Bytes*>() == target._getValue<Bytes*>();
			return false;
		}
		case Type::SHORT_BYTES_LE_15:
		{
			if (target._type == Type::SHORT_BYTES_LE_15) return _value[VALUE_SIZE - 1] == target._value[VALUE_SIZE - 1] && !memcmp(_value, target._value, _value[VALUE_SIZE - 1]);
			return false;
		}
		case Type::SHORT_BYTES16:
		{
			if (target._type == Type::SHORT_BYTES16) return !memcmp(_value, target._value, VALUE_SIZE);
			return false;
		}
		case Type::EXT_BYTES:
		{
			if (target._type == Type::EXT_BYTES) return _getValue<BytesView>() == target._getValue<BytesView>();
			return false;
		}
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
			return _equal<int64_t>(target);
		case Type::UINT:
			return _equal<uint64_t>(target);
		case Type::FLOAT32:
			return _equal<float32_t>(target);
		case Type::FLOAT64:
			return _equal<float64_t>(target);
		case Type::STRING:
		{
			switch (target._type) {
			case Type::STRING:
			{
				auto s1 = _getValue<Str*>();
				auto s2 = target._getValue<Str*>();
				return _isContentEqual(s1->data, s1->size, s2->data, s2->size);
			}
			case Type::SHORT_STRING:
			{
				auto s = _getValue<Str*>();
				return _isContentEqual(s->data, s->size, target._value, strlen((char*)target._value));
			}
			case Type::STRING_VIEW:
			{
				auto s = _getValue<Str*>();
				auto& sv = target._getValue<StrView>();
				return _isContentEqual(s->data, s->size, sv.data, sv.size);
			}
			default:
				return false;
			}
		}
		case Type::SHORT_STRING:
		{
			switch (target._type) {
			case Type::STRING:
			{
				auto s = target._getValue<Str*>();
				return _isContentEqual(s->data, s->size, _value, strlen((char*)_value));
			}
			case Type::SHORT_STRING:
				return _isContentEqual(_value, strlen((char*)_value), target._value, strlen((char*)target._value));
			case Type::STRING_VIEW:
			{
				auto& sv = target._getValue<StrView>();
				return _isContentEqual(_value, strlen((char*)_value), sv.data, sv.size);
			}
			default:
				return false;
			}
		}
		case Type::STRING_VIEW:
		{
			switch (target._type) {
			case Type::STRING:
			{
				auto s = target._getValue<Str*>();
				auto& sv = _getValue<StrView>();
				return _isContentEqual(s->data, s->size, sv.data, sv.size);
			}
			case Type::SHORT_STRING:
			{
				auto sv = _getValue<StrView>();
				return _isContentEqual(sv.data, sv.size, target._value, strlen((char*)target._value));
			}
			case Type::STRING_VIEW:
			{
				auto& sv1 = _getValue<StrView>();
				auto& sv2 = target._getValue<StrView>();
				return _isContentEqual(sv1.data, sv1.size, sv2.data, sv2.size);
			}
			default:
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
				auto val1 = _getValue<Bytes*>();
				auto val2 = target._getValue<Bytes*>();
				return val1->getSize() == val2->getSize() && !memcmp(val1->getValue(), val2->getValue(), val1->getSize());
			} else if (target._type == Type::EXT_BYTES) {
				auto val1 = _getValue<Bytes*>();
				auto& val2 = target._getValue<BytesView>();
				return val1->getSize() == val2.size && !memcmp(val1->getValue(), val2.data, val2.size);
			} else {
				return false;
			}
		}
		case Type::SHORT_BYTES_LE_15:
		{
			if (target._type == Type::SHORT_BYTES_LE_15) {
				return _value[VALUE_SIZE - 1] == target._value[VALUE_SIZE - 1] && !memcmp(_value, target._value, _value[VALUE_SIZE - 1]);
			} else if (target._type == Type::EXT_BYTES) {
				auto& val2 = target._getValue<BytesView>();
				return _value[VALUE_SIZE - 1] == val2.size && !memcmp(_value, val2.data, val2.size);
			} else {
				return false;
			}
		}
		case Type::SHORT_BYTES16:
		{
			if (target._type == Type::SHORT_BYTES16) {
				return !memcmp(_value, target._value, VALUE_SIZE);
			} else if (target._type == Type::EXT_BYTES) {
				auto& val2 = target._getValue<BytesView>();
				return VALUE_SIZE == val2.size && !memcmp(_value, val2.data, VALUE_SIZE);
			} else {
				return false;
			}
		}
		case Type::EXT_BYTES:
		{
			switch (target._type) {
			case Type::BYTES:
			{
				auto& val1 = _getValue<BytesView>();
				auto val2 = target._getValue<Bytes*>();
				return val1.size == val2->getSize() && !memcmp(val1.data, val2->getValue(), val1.size);
			}
			case Type::SHORT_BYTES_LE_15:
			{
				auto& val1 = _getValue<BytesView>();
				return val1.size == target._value[VALUE_SIZE - 1] && !memcmp(val1.data, target._value, val1.size);
			}
			case Type::SHORT_BYTES16:
			{
				auto& val1 = _getValue<BytesView>();
				return val1.size == VALUE_SIZE && !memcmp(val1.data, target._value, VALUE_SIZE);
			}
			case Type::EXT_BYTES:
			{
				auto& val1 = _getValue<BytesView>();
				auto& val2 = target._getValue<BytesView>();
				return val1.size == val2.size && !memcmp(val1.data, val2.data, val1.size);
			}
			default:
				return false;
			}
		}
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
			return _getValue<Bytes*>()->getSize();
		case Type::EXT_BYTES:
			return _getValue<BytesView>().size;
		case Type::SHORT_BYTES_LE_15:
			return _value[VALUE_SIZE - 1];
		case Type::SHORT_BYTES16:
			return VALUE_SIZE;
		case Type::STRING:
			return _getValue<Str*>()->size;
		case Type::SHORT_STRING:
			return strlen((char*)_value);
		case Type::STRING_VIEW:
			return _getValue<StrView>().size;
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
			_getValue<Bytes*>()->clear();
			break;
		case Type::EXT_BYTES:
			_getValue<BytesView>().size = 0;
			break;
		case Type::STRING:
		{
			intrusivePtrRelease(*_getValue<Str*>());
			_value[0] = 0;
			_type = Type::SHORT_STRING;

			break;
		}
		case Type::SHORT_STRING:
			_value[0] = 0;
			break;
		case Type::STRING_VIEW:
		{
			auto& sv = _getValue<StrView>();
			sv.data = nullptr;
			sv.size = 0;

			break;
		}
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
		case Type::FLOAT32:
			return _getValue<float32_t>() != 0.0f;
		case Type::FLOAT64:
			return _getValue<float64_t>() != 0.0;
		default:
			return defaultValue;
		}
	}

	std::string SerializableObject::toString(const std::string& defaultValue) const {
		switch (_type) {
		case Type::BOOL:
			return _getValue<bool>() ? "true" : "false";
		case Type::INT:
			return StringUtility::toString<int64_t>(_getValue<int64_t>());
		case Type::UINT:
			return StringUtility::toString<uint64_t>(_getValue<uint64_t>());
		case Type::FLOAT32:
			return StringUtility::toString<float32_t>(_getValue<float32_t>());
		case Type::FLOAT64:
			return StringUtility::toString<float64_t>(_getValue<float64_t>());
		case Type::STRING:
		{
			auto s = _getValue<Str*>();
			return std::string(s->data, s->size);
		}
		case Type::SHORT_STRING:
			return (char*)_value;
		case Type::STRING_VIEW:
		{
			auto& sv = _getValue<StrView>();
			return std::string(sv.data, sv.size);
		}
		case Type::INVALID:
			return "";
		default:
			return defaultValue;
		}
	}

	std::string_view SerializableObject::toStringView() const {
		switch (_type) {
		case Type::STRING:
		{
			auto s = _getValue<Str*>();
			return std::string_view(s->data, s->size);
		}
		case Type::SHORT_STRING:
			return std::string_view((char*)_value);
		case Type::STRING_VIEW:
		{
			auto& sv = _getValue<StrView>();
			return std::string_view(sv.data, sv.size);
		}
		default:
			return std::string_view();
		}
	}

	const uint8_t* SerializableObject::toBytes() const {
		switch (_type) {
		case Type::BYTES:
			return _getValue<Bytes*>()->getValue();
		case Type::EXT_BYTES:
			return _getValue<BytesView>().data;
		case Type::SHORT_BYTES_LE_15:
		case Type::SHORT_BYTES16:
			return _value;
		default:
			return nullptr;
		}
	}

	void SerializableObject::_toJson(std::string& json) const {
		switch (_type) {
		case Type::INVALID:
			json += "\"$invalid\"";
			break;
		case Type::BOOL:
		case Type::INT:
		case Type::UINT:
		case Type::FLOAT32:
		case Type::FLOAT64:
			json += toString();
			break;
		case Type::STRING:
		case Type::SHORT_STRING:
		case Type::STRING_VIEW:
		{
			json += '"';
			json += toStringView();
			json += '"';

			break;
		}
		case Type::ARRAY:
		{
			json += '[';

			auto& arr = _getValue<Array*>()->value;
			auto first = true;
			for (auto& i : arr) {
				if (first) {
					first = false;
				} else {
					json += ',';
				}
				i.wrap()._toJson(json);
			}

			json += ']';

			break;
		}
		case Type::MAP:
		{
			json += '{';

			auto& map = _getValue<Map*>()->value;
			auto first = true;
			for (auto& itr : map) {
				if (first) {
					first = false;
				} else {
					json += ',';
				}
				itr.first.wrap()._toJson(json);
				json += ':';
				itr.second.wrap()._toJson(json);
			}

			json += '}';

			break;
		}
		case Type::BYTES:
		{
			auto bin = _getValue<Bytes*>();
			json += "\"$binary_" + StringUtility::toString(bin->getValue(), bin->getSize()) + "\"";

			break;
		}
		case Type::EXT_BYTES:
		{
			auto& bin = _getValue<BytesView>();
			json += "\"$binary_" + StringUtility::toString(bin.data, bin.size) + "\"";

			break;
		}
		default:
			break;
		}
	}

	void SerializableObject::set(const std::string_view& value, Flag flag) {
		using namespace srk::enum_operators;

		_flag = Flag::NONE;
		switch (_type) {
		case Type::STRING:
		{
			intrusivePtrRelease(*_getValue<Str*>());

			if ((flag & Flag::COPY) == Flag::COPY) {
				if (auto size = value.size(); size < VALUE_SIZE) {
					_writeShortString(value.data(), size);
					_type = Type::SHORT_STRING;
				} else {
					_getValue<Str*>() = new Str(value.data(), size);
				}
			} else {
				_writeStringView(value.data(), value.size());
				_type = Type::STRING_VIEW;
				_flag = flag & Flag::TO_COPY;
			}

			break;
		}
		case Type::SHORT_STRING:
		{
			if ((flag & Flag::COPY) == Flag::COPY) {
				if (auto size = value.size(); size < VALUE_SIZE) {
					_writeShortString(value.data(), size);
				} else {
					_getValue<Str*>() = new Str(value.data(), size);
					_type = Type::STRING;
				}
			} else {
				_writeStringView(value.data(), value.size());
				_type = Type::STRING_VIEW;
				_flag = flag & Flag::TO_COPY;
			}

			break;
		}
		case Type::STRING_VIEW:
		{
			if ((flag & Flag::COPY) == Flag::COPY) {
				if (auto size = value.size(); size < VALUE_SIZE) {
					_writeShortString(value.data(), size);
					_type = Type::SHORT_STRING;
				} else {
					_getValue<Str*>() = new Str(value.data(), size);
					_type = Type::STRING;
				}
			} else {
				_writeStringView(value.data(), value.size());
				_flag = flag & Flag::TO_COPY;
			}

			break;
		}
		default:
		{
			_freeValue();

			if ((flag & Flag::COPY) == Flag::COPY) {
				if (auto size = value.size(); size < VALUE_SIZE) {
					_writeShortString(value.data(), size);
					_type = Type::SHORT_STRING;
				} else {
					_getValue<Str*>() = new Str(value.data(), size);
					_type = Type::STRING;
				}
			} else {
				_writeStringView(value.data(), value.size());
				_type = Type::STRING_VIEW;
				_flag = flag & Flag::TO_COPY;
			}

			break;
		}
		}
	}

	void SerializableObject::set(const void* value, size_t size, Flag flag) {
		using namespace srk::enum_operators;

		_flag = Flag::NONE;
		if ((flag & Flag::COPY) == Flag::COPY) {
			if (size < VALUE_SIZE) {
				memcpy(_value, value, size);
				if (size < VALUE_SIZE) {
					_value[VALUE_SIZE - 1] = size;
					_type = Type::SHORT_BYTES_LE_15;
				} else {
					_type = Type::SHORT_BYTES16;
				}
			} else {
				setBytes<false, false>();
				_getValue<Bytes*>()->setValue((const uint8_t*)value, size);
			}
		} else {
			setBytes<true>();
			auto& val = _getValue<BytesView>();
			val.data = (const uint8_t*)value;
			val.size = size;
		}
	}

	void SerializableObject::set(const SerializableObject& value, Flag flag) {
		using namespace srk::enum_operators;

		_flag = Flag::NONE;
		switch (value._type) {
		case Type::STRING:
		{
			_freeValue();
			_type = Type::STRING;

			_getValue<Str*>() = (value._getValue<Str*>())->ref<Str>();

			break;
		}
		case Type::SHORT_STRING:
		{
			_freeValue();
			_type = Type::SHORT_STRING;

			memcpy(_value, value._value, strlen((const char*)value._value));

			break;
		}
		case Type::STRING_VIEW:
		{
			auto& sv = value._getValue<StrView>();
			set(std::string_view(sv.data, sv.size), flag | ((value._flag & Flag::TO_COPY) != Flag::NONE ? Flag::COPY : Flag::NONE));

			break;
		}
		case Type::ARRAY:
		{
			_freeValue();
			_type = Type::ARRAY;

			auto arr = value._getValue<Array*>();
			_getValue<Array*>() = (flag & (Flag::COPY | Flag::DETACH_COPY)) != Flag::NONE ? arr->copy(flag) : arr->ref<Array>();

			break;
		}
		case Type::MAP:
		{
			_freeValue();
			_type = Type::MAP;

			auto map = value._getValue<Map*>();
			_getValue<Map*>() = (flag & (Flag::COPY | Flag::DETACH_COPY)) != Flag::NONE ? map->copy(flag) : map->ref<Map>();

			break;
		}
		case Type::BYTES:
		{
			_freeValue();
			_type = Type::BYTES;

			auto bytes = value._getValue<Bytes*>();
			_getValue<Bytes*>() = bytes->ref<Bytes>();

			break;
		}
		case Type::EXT_BYTES:
		{
			if ((flag & Flag::COPY) != Flag::NONE || (value._flag & Flag::TO_COPY) != Flag::NONE) {
				_getValue<Bytes*>() = value._getValue<Bytes*>()->copy();
				_type = Type::BYTES;
			} else {
				memcpy(_value, value._value, VALUE_SIZE);
			}

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
			return index < arr->value.size() ? arr->value[index].wrap() : std::move(SerializableObject());
		} else {
			return std::move(SerializableObject());
		}
	}

	SerializableObject* SerializableObject::tryAtPtr(size_t index) const {
		if (_type == Type::ARRAY) {
			Array* arr = _getValue<Array*>();
			return index < arr->value.size() ? &arr->value[index].wrap() : nullptr;
		} else {
			return nullptr;
		}
	}

	SerializableObject& SerializableObject::push() {
		return _getArray()->value.emplace_back();
	}

	SerializableObject& SerializableObject::push(const SerializableObject& value) {
		return _getArray()->value.emplace_back(value);
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

		return map->value.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple()).first->second;
	}

	SerializableObject SerializableObject::tryGet(const SerializableObject& key) const {
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

			auto itr = map->value.find((const SerializableObjectWrapper&)key);
			return itr == map->value.end() ? std::move(SerializableObject()) : itr->second.wrap();
		} else {
			return std::move(SerializableObject());
		}
	}

	SerializableObject* SerializableObject::tryGetPtr(const SerializableObject& key) const {
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

			auto itr = map->value.find((const SerializableObjectWrapper&)key);
			return itr == map->value.end() ? nullptr : &itr->second.wrap();
		} else {
			return nullptr;
		}
	}

	SerializableObject& SerializableObject::insert(const SerializableObject& key, const SerializableObject& value) {
		Map* map = _getMap();

		return map->value.insert_or_assign(key, value).first->second.wrap();
	}

	bool SerializableObject::has(const SerializableObject& key) const {
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

			return map->value.find((const SerializableObjectWrapper&)key) != map->value.end();
		} else {
			return false;
		}
	}

	SerializableObject SerializableObject::remove(const SerializableObject& key) {
		if (_type == Type::MAP) {
			Map* map = _getValue<Map*>();

			auto itr = map->value.find((const SerializableObjectWrapper&)key);
			if (itr == map->value.end()) {
				return std::move(SerializableObject());
			} else {
				SerializableObject v = itr->second.wrap();
				map->value.erase(itr);
				return std::move(v);
			}
		} else {
			return std::move(SerializableObject());
		}
	}

	void SerializableObject::_packUInt(ByteArray& ba, uint64_t val, uint8_t typeBegin, uint8_t typeEnd) const {
		if (val <= Bit::uintMax<8>()) {
			ba.write<ba_vt::UI8>(typeBegin | typeEnd);
			ba.write<ba_vt::UI8>(val);
		} else if (val <= Bit::uintMax<16>()) {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 1));
			ba.write<ba_vt::UI16>(val);
		} else if (val <= Bit::uintMax<24>()) {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 2));
			ba.write<ba_vt::UIX>(val, 3);
		} else if (val <= Bit::uintMax<32>()) {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 3));
			ba.write<ba_vt::UI32>(val);
		} else if (val <= Bit::uintMax<40>()) {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 4));
			ba.write<ba_vt::UIX>(val, 5);
		} else if (val <= Bit::uintMax<48>()) {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 5));
			ba.write<ba_vt::UIX>(val, 6);
		} else if (val <= Bit::uintMax<56>()) {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 6));
			ba.write<ba_vt::UIX>(val, 7);
		} else {
			ba.write<ba_vt::UI8>(typeBegin | (typeEnd + 7));
			ba.write<ba_vt::UI64>(val);
		}
	}

	void SerializableObject::_packString(ByteArray& ba, const char* data, size_t size) const {
		if (size < VAL5_MAX) {
			ba.write<uint8_t>(((uint8_t)Type::STRING << 5) | size);
			ba.write<ba_vt::BYTE>(data, size);
		} else {
			ba.write<uint8_t>(((uint8_t)Type::STRING << 5) | VAL5_MAX);
			ba.write<ba_vt::STR>(data, size);
		}
	}

	void SerializableObject::_packBytes(ByteArray& ba, const uint8_t* data, size_t size) const {
		if (size < VAL5_BITS8) {
			ba.write<uint8_t>(((uint8_t)Type::BYTES << 5) | size);
		} else {
			_packUInt(ba, size, (uint8_t)Type::BYTES << 5, VAL5_BITS8);
		}

		ba.write<ba_vt::BYTE>(data, size);
	}

	void SerializableObject::unpack(ByteArray& ba, Flag flag) {
		if (ba.getBytesAvailable() > 0) {
			auto tval = ba.read<uint8_t>();
			auto type = (Type)(tval >> 5 & 0b111);
			auto val = tval & 0b11111;
			switch (type) {
			case Type::INVALID:
			{
				switch (tval) {
				case BOOL_FALSE:
					set(false);
					break;
				case BOOL_TRUE:
					set(true);
					break;
				default:
					setInvalid();
					break;
				}

				break;
			}
			case Type::INT:
			{
				auto sign = (val & 0b10000) != 0;
				val &= 0b1111;
				if (val >= VAL4_BITS8) {
					if (sign) {
						set(-(int64_t)ba.read<ba_vt::UIX>(val - VAL4_BITS8 + 1));
					} else {
						set(ba.read<ba_vt::UIX>(val - VAL4_BITS8 + 1));
					}
				} else {
					if (sign) {
						set(-val);
					} else {
						set(val);
					}
				}

				break;
			}
			case Type::FLOAT:
			{
				if ((val & 0b10000) == 0) {
					float32_t v;
					auto p = (uint8_t*)&v;

					for (size_t i = 0; i < 4; ++i) {
						if (val & (0b1 << i)) {
							p[i] = ba.read<uint8_t>();
						} else {
							p[i] = 0;
						}
					}

					set(v);
				} else {
					float64_t v;
					auto p = (uint8_t*)&v;

					for (size_t i = 0; i < 4; ++i) {
						auto ii = i << 1;
						if (val & (0b1 << i)) {
							p[ii] = ba.read<uint8_t>();
							p[++ii] = ba.read<uint8_t>();
						} else {
							p[ii] = 0;
							p[++ii] = 0;
						}
					}

					set(v);
				}

				break;
			}
			case Type::STRING:
			{
				if (val == VAL5_MAX) {
					set(ba.read<std::string_view>(), flag);
				} else {
					set(std::string_view((const char*)ba.getCurrentSource(), val), flag);
					ba.skip(val);
				}

				break;
			}
			case Type::ARRAY:
			{
				auto& arr = _getArray()->value;

				if (val == VAL5_MAX) {
					arr.clear();
					while (ba.getBytesAvailable()) {
						if (*ba.getCurrentSource() == END) {
							ba.skip(1);
							break;
						} else {
							arr.emplace_back().wrap().unpack(ba, flag);
						}
					}
				} else {
					arr.resize(val);
					for (size_t i = 0; i < val; ++i) arr[i].wrap().unpack(ba, flag);
				}

				break;
			}
			case Type::MAP:
			{
				if (val == VAL5_MAX) {
					auto& map = _getMap()->value;
					map.clear();
					SerializableObject k, v;
					while (ba.getBytesAvailable()) {
						if (*ba.getCurrentSource() == END) {
							ba.skip(1);
							break;
						} else {
							k.unpack(ba, flag);
							v.unpack(ba, flag);
							map.emplace((SerializableObjectWrapper&&)std::move(k), (SerializableObjectWrapper&&)std::move(v));
						}
					}
				} else {
					_getMap()->unpack(ba, val, flag);
				}

				break;
			}
			case Type::BYTES:
			{
				if (val >= VAL5_BITS8) {
					_unpackBytes(ba, ba.read<ba_vt::UIX>(val - VAL5_BITS8 + 1), flag);
				} else {
					_unpackBytes(ba, val, flag);
				}

				break;
			}
			default:
				break;
			}
		} else {
			setInvalid();
		}
	}

	void SerializableObject::_unpackBytes(ByteArray& ba, size_t size, Flag flag) {
		using namespace srk::enum_operators;

		_freeValue();
		if ((flag & Flag::COPY) == Flag::COPY) {
			if (size <= VALUE_SIZE) {
				memcpy(_value, ba.getCurrentSource(), size);
				if (size < VALUE_SIZE) {
					_value[VALUE_SIZE - 1] = size;
					_type = Type::SHORT_BYTES_LE_15;
				} else {
					_type = Type::SHORT_BYTES16;
				}
			} else {
				_getValue<Bytes*>() = new Bytes(ba.getCurrentSource(), size);
				_type = Type::BYTES;
			}
		} else {
			auto& val = _getValue<BytesView>();
			val.data = ba.getCurrentSource();
			val.size = size;
			_type = Type::EXT_BYTES;
		}
		ba.skip(size);
	}

	void SerializableObject::_freeValue() {
		switch (_type) {
		case Type::STRING:
			intrusivePtrRelease(*_getValue<Str*>());
			break;
		case Type::ARRAY:
			intrusivePtrRelease(*_getValue<Array*>());
			break;
		case Type::MAP:
			intrusivePtrRelease(*_getValue<Map*>());
			break;
		case Type::BYTES:
			intrusivePtrRelease(*_getValue<Bytes*>());
			break;
		case Type::EXT_BYTES:
			break;
		default:
			break;
		}

		_type = Type::INVALID;
		_flag = Flag::NONE;
	}
}