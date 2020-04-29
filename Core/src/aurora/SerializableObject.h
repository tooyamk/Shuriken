#pragma once

#include "aurora/Ref.h"
#include "aurora/ByteArray.h"
#include "aurora/String.h"
#include <unordered_map>
#include <functional>

namespace aurora {
	class SerializationValueMapElement;

	class AE_DLL SerializableObject {
	public:
		class IPackFilter {
		public:
			virtual bool AE_CALL packable(const SerializableObject* parent, size_t depth, size_t index, const SerializableObject& val) const = 0;
			virtual bool AE_CALL packable(const SerializableObject* parent, size_t depth, const SerializableObject& key, const SerializableObject& val) const = 0;
		};


		enum class Type : uint8_t {
			INVALID,

			BOOL,

			INT,
			UINT,

			FLOAT32,
			FLOAT64,

			STRING,
			ARRAY,
			MAP,

			BYTES,
			EXT_BYTES,

			SHORT_STRING,
			STD_SV
		};


		enum class ForEachOperation : uint8_t {
			CONTINUE,
			BREAK,
			ERASE
		};


		struct std_unordered_hasher {
			inline size_t AE_CALL operator()(const SerializableObject& value) const {
				switch (value._type) {
				case Type::BOOL:
					return std::hash<bool>{}(value._getValue<bool>());
				case Type::BYTES:
					return std::hash<Bytes<false>*>{}(value._getValue<Bytes<false>*>());
				case Type::EXT_BYTES:
					return std::hash<Bytes<true>*>{}(value._getValue<Bytes<true>*>());
				case Type::FLOAT32:
					return std::hash<float32_t>{}(value._getValue<float32_t>());
				case Type::FLOAT64:
					return std::hash<float64_t>{}(value._getValue<float64_t>());
				case Type::INT:
				case Type::UINT:
					return std::hash<uint64_t>{}(value._getValue<uint64_t>());
				case Type::ARRAY:
					return std::hash<Array*>{}(value._getValue<Array*>());
				case Type::MAP:
					return std::hash<Map*>{}(value._getValue<Map*>());
				case Type::STRING:
				{
					auto str = value._getValue<Str*>();
					return std::hash<std::string_view>{}(std::string_view(str->value, str->size));
				}
				case Type::SHORT_STRING:
					return std::hash<std::string_view>{}(std::string_view((char*)value._value));
				case Type::STD_SV:
					return std::hash<std::string_view>{}(*value._getValue<std::string_view*>());
				default:
					return 0;
				}
			}
		};


		struct std_unordered_compare {
			inline bool AE_CALL operator()(const SerializableObject& value1, const SerializableObject& value2) const {
				return value1.isEqual(value2);
			}
		};


		SerializableObject();
		SerializableObject(Type value);
		SerializableObject(bool value);
		SerializableObject(int8_t value);
		SerializableObject(uint8_t value);
		SerializableObject(int16_t value);
		SerializableObject(uint16_t value);
		SerializableObject(int32_t value);
		SerializableObject(uint32_t value);
		SerializableObject(const int64_t& value);
		SerializableObject(const uint64_t& value);
		SerializableObject(float32_t value);
		SerializableObject(const float64_t& value);
		SerializableObject(const char* value);
		SerializableObject(const std::string& value);
		SerializableObject(const std::string_view& value);
		SerializableObject(const uint8_t* value, size_t size, bool copy);
		SerializableObject(ByteArray& ba, bool copy);
		SerializableObject(const SerializableObject& key, const SerializableObject& value);
		SerializableObject(const SerializableObject& value);
		SerializableObject(const SerializableObject& value, bool copy);
		SerializableObject(SerializableObject&& value);
		~SerializableObject();

		inline SerializableObject& AE_CALL operator=(const SerializableObject& value) {
			set(value);
			return *this;
		}
		inline SerializableObject& AE_CALL operator=(SerializableObject&& value) noexcept {
			_type = value._type;
			memcpy(_value, value._value, VALUE_SIZE);
			value._type = Type::INVALID;

			return *this;
		}

		inline Type AE_CALL getType() const {
			return _type == Type::SHORT_STRING ? Type::STRING : _type;
		}
		inline bool AE_CALL isValid() const {
			return _type != Type::INVALID;
		}
		inline bool AE_CALL isNumber() const {
			return _type == Type::INT || _type == Type::UINT || _type == Type::FLOAT32 || _type == Type::FLOAT64;
		}
		inline bool AE_CALL isInteger() const {
			return _type == Type::INT || _type == Type::UINT;
		}
		inline bool AE_CALL isFloatingPoint() const {
			return _type == Type::FLOAT32 || _type == Type::FLOAT64;
		}
		inline bool AE_CALL isBytes() const {
			return _type == Type::BYTES || _type == Type::EXT_BYTES;
		}

		size_t AE_CALL getSize() const;
		void AE_CALL clear();

		bool AE_CALL setInvalid();
		bool AE_CALL setArray();
		bool AE_CALL setMap();

		template<bool Ext>
		bool AE_CALL setBytes() {
			if constexpr (Ext) {
				if (_type == Type::EXT_BYTES) {
					return false;
				} else {
					_freeValue();

					_getValue<Bytes<true>*>() = new Bytes<Ext>();
					_type = Type::EXT_BYTES;

					return true;
				}
			} else {
				if (_type == Type::BYTES) {
					return false;
				} else {
					_freeValue();

					_getValue<Bytes<false>*>() = new Bytes<Ext>();
					_type = Type::BYTES;

					return true;
				}
			}
		}

		bool AE_CALL toBool(bool defaultValue = false) const;

		template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
		T AE_CALL toNumber(T defaultValue = 0) const {
			switch (_type) {
			case Type::BOOL:
				return _getValue<bool>()? 1 : 0;
			case Type::INT:
				return _getValue<int64_t>();
			case Type::UINT:
				return _getValue<uint64_t>();
			case Type::FLOAT32:
				return _getValue<float32_t>();
			case Type::FLOAT64:
				return _getValue<float64_t>();
			case Type::STRING:
			{
				auto s = _getValue<Str*>();
				return String::toNumber<T>(std::string_view(s->value, s->size));
			}
			case Type::SHORT_STRING:
				return String::toNumber<T>(std::string_view((char*)_value, strlen((char*)_value)));
			default:
				return defaultValue;
			}
		}

		std::string AE_CALL toString(const std::string& defaultValue = "") const;
		std::string_view AE_CALL toStringView() const;
		const uint8_t* AE_CALL toBytes() const;
		inline std::string AE_CALL toJson() const {
			std::string json;
			_toJson(json);
			return std::move(json);
		}

		template<typename T>
		T AE_CALL toValue() {
			if constexpr (std::is_arithmetic_v<T>) {
				return toNumber<T>();
			} else if constexpr (std::is_base_of_v<std::string, T>) {
				return toString();
			} else if constexpr (std::is_base_of_v<std::string_view, T>) {
				return toStringView();
			} else if constexpr (std::is_base_of_v<bool, T>) {
				return toBool();
			} else {
				return (T)0;
			}
		}

		inline void AE_CALL set(bool value) {
			_set<bool, Type::BOOL>(value);
		}
		inline void AE_CALL set(int8_t value) {
			_setInt(value);
		}
		inline void AE_CALL set(uint8_t value) {
			_setUInt(value);
		}
		inline void AE_CALL set(int16_t value) {
			_setInt(value);
		}
		inline void AE_CALL set(uint16_t value) {
			_setUInt(value);
		}
		inline void AE_CALL set(int32_t value) {
			_setInt(value);
		}
		inline void AE_CALL set(uint32_t value) {
			_setUInt(value);
		}
		inline void AE_CALL set(const int64_t& value) {
			_setInt(value);
		}
		inline void AE_CALL set(const uint64_t& value) {
			_setUInt(value);
		}
		inline void AE_CALL set(float32_t value) {
			_set<float32_t, Type::FLOAT32>(value);
		}
		inline void AE_CALL set(const float64_t& value) {
			_set<float64_t, Type::FLOAT64>(value);
		}
		inline void AE_CALL set(const char* value) {
			set(std::string_view(value, strlen(value)));
		}
		inline void AE_CALL set(const std::string& value) {
			set(std::string_view(value));
		}
		void AE_CALL set(const std::string_view& value);
		void AE_CALL set(const uint8_t* value, size_t size, bool copy);

		void AE_CALL set(const SerializableObject& value, bool copy = false);

		SerializableObject& AE_CALL at(size_t index);
		SerializableObject AE_CALL tryAt(size_t index) const;
		SerializableObject& AE_CALL push();
		SerializableObject& AE_CALL push(const SerializableObject& value);
		SerializableObject AE_CALL removeAt(size_t index);
		void AE_CALL insertAt(size_t index, const SerializableObject& value);

		SerializableObject& AE_CALL get(const SerializableObject& key);
		inline SerializableObject& AE_CALL get(const char* key) {
			return get(std::string_view(key, strlen(key)));
		}
		inline SerializableObject& AE_CALL get(const std::string& key) {
			return get(std::string_view(key));
		}
		inline SerializableObject& AE_CALL get(const std::string_view& key) {
			SerializableObject so;
			so._type = Type::STD_SV;
			so._getValue<const std::string_view*>() = &key;
			return get(so);
		}
		SerializableObject AE_CALL tryGet(const SerializableObject& key) const;
		inline SerializableObject AE_CALL tryGet(const char* key) const {
			return tryGet(std::string_view(key, strlen(key)));
		}
		inline SerializableObject AE_CALL tryGet(const std::string& key) const {
			return tryGet(std::string_view(key));
		}
		inline SerializableObject AE_CALL tryGet(const std::string_view& key) const {
			SerializableObject so;
			so._type = Type::STD_SV;
			so._getValue<const std::string_view*>() = &key;
			return tryGet(so);
		}
		SerializableObject& AE_CALL insert(const SerializableObject& key, const SerializableObject& value);
		inline SerializableObject& AE_CALL insert(const char* key, const SerializableObject& value) {
			return insert(std::string_view(key, strlen(key)), value);
		}
		inline SerializableObject& AE_CALL insert(const std::string& key, const SerializableObject& value) {
			return insert(std::string_view(key), value);
		}
		inline SerializableObject& AE_CALL insert(const std::string_view& key, const SerializableObject& value) {
			SerializableObject so;
			so._type = Type::STD_SV;
			so._getValue<const std::string_view*>() = &key;
			return insert(so, value);
		}
		SerializableObject AE_CALL remove(const SerializableObject& key);
		inline SerializableObject AE_CALL remove(const char* key) {
			return remove(std::string_view(key, strlen(key)));
		}
		inline SerializableObject AE_CALL remove(const std::string& key) {
			return remove(std::string_view(key));
		}
		inline SerializableObject AE_CALL remove(const std::string_view& key) {
			SerializableObject so;
			so._type = Type::STD_SV;
			so._getValue<const std::string_view*>() = &key;
			return remove(so);
		}
		bool AE_CALL has(const SerializableObject& key) const;
		inline bool AE_CALL has(const char* key) const {
			return has(std::string_view(key, strlen(key)));
		}
		inline bool AE_CALL has(const std::string& key) const {
			return has(std::string_view(key));
		}
		inline bool AE_CALL has(const std::string_view& key) const {
			SerializableObject so;
			so._type = Type::STD_SV;
			so._getValue<const std::string_view*>() = &key;
			return has(so);
		}

		template<typename Fn, typename = 
			std::enable_if_t<std::is_invocable_v<Fn, const SerializableObject&, const SerializableObject&> &&
			(std::is_same_v<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, void> ||
			std::is_same_v<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, bool>), Fn>>
		void AE_CALL forEach(const Fn& fn) const {
			if (_type == Type::ARRAY) {
				if (Array* arr = _getValue<Array*>(); arr) {
					SerializableObject idx;
					for (size_t i = 0, n = arr->value.size(); i < n; ++i) {
						idx.set(i);
						if constexpr (std::is_same_v<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, void>) {
							fn(idx, arr->value[i]);
						} else {
							if (!fn(idx, arr->value[i])) break;
						}
					}
				}
			} else if (_type == Type::MAP) {
				if (Map* map = _getValue<Map*>(); map) {
					for (auto& itr : map->value) {
						if constexpr (std::is_same_v<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, void>) {
							fn(itr.first, itr.second);
						} else {
							if (!fn(itr.first, itr.second)) break;
						}
					}
				}
			}
		}
		
		template<typename Fn, typename = 
			std::enable_if_t<std::is_invocable_v<Fn, const SerializableObject&, const SerializableObject&> &&
			std::is_same_v<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, ForEachOperation>, Fn>>
		void AE_CALL forEach(const Fn& fn) {
			if (_type == Type::ARRAY) {
				if (Array* arr = _getValue<Array*>(); arr) {
					SerializableObject idx;
					for (size_t i = 0, n = arr->value.size(); i < n; ++i) {
						idx.set(i);
						auto op = fn(idx, arr->value[i]);
						if ((op & ForEachOperation::ERASE) == ForEachOperation::ERASE) {
							arr->value.erase(arr->value.begin() + i);
							--i;
							--n;
						}
						if ((op & ForEachOperation::BREAK) == ForEachOperation::BREAK) break;
					}
				}
			} else if (_type == Type::MAP) {
				if (Map* map = _getValue<Map*>(); map) {
					for (auto itr = map->value.begin(); itr != map->value.end();) {
						auto op = fn(itr->first, itr->second);
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

		template<typename T = std::nullptr_t, typename = std::enable_if_t<std::is_null_pointer_v<T> || std::is_base_of_v<IPackFilter, T>, T>>
		inline void AE_CALL pack(ByteArray& ba, const T& filter = nullptr) const {
			_pack(nullptr, 0, ba, filter);
		}
		void AE_CALL unpack(ByteArray& ba);

		bool AE_CALL isEqual(const SerializableObject& target) const;
		bool AE_CALL isContentEqual(const SerializableObject& target) const;

		inline bool operator==(const SerializableObject& right) const {
			return isEqual(right);
		}

	private:
		enum class InternalType : uint8_t {
			UNVALID = (uint8_t)Type::INVALID,

			BOOL = (uint8_t)Type::BOOL,
			INT = (uint8_t)Type::INT,
			UINT = (uint8_t)Type::UINT,

			FLOAT = (uint8_t)Type::FLOAT32,
			DOUBLE = (uint8_t)Type::FLOAT64,

			STRING = (uint8_t)Type::STRING,
			ARRAY = (uint8_t)Type::ARRAY,
			MAP = (uint8_t)Type::MAP,

			BYTES = (uint8_t)Type::BYTES,
			EXT_BYTES = (uint8_t)Type::EXT_BYTES,

			SHORT_STRING = (uint8_t)Type::SHORT_STRING,

			BOOL_TRUE = 18,
			BOOL_FALSE,

			N_INT_1 = 20,
			N_INT_8BITS,
			N_INT_16BITS,
			N_INT_24BITS,
			N_INT_32BITS,
			N_INT_40BITS,
			N_INT_48BITS,
			N_INT_56BITS,
			N_INT_64BITS,
			P_INT_0 = 40,
			P_INT_1,
			P_INT_8BITS,
			P_INT_16BITS,
			P_INT_24BITS,
			P_INT_32BITS,
			P_INT_40BITS,
			P_INT_48BITS,
			P_INT_56BITS,
			P_INT_64BITS,
			FLT_0 = 60,
			FLT_0_5,
			FLT_1,
			DBL_0 = 80,
			DBL_0_5,
			DBL_1,
			STRING_EMPTY = 100,
			ARRAY_0 = 120,
			ARRAY_8BITS,
			ARRAY_16BITS,
			ARRAY_24BITS,
			ARRAY_32BITS,
			ARRAY_40BITS,
			ARRAY_48BITS,
			ARRAY_56BITS,
			ARRAY_64BITS,
			ARRAY_END,
			MAP_0 = 140,
			MAP_8BITS,
			MAP_16BITS,
			MAP_24BITS,
			MAP_32BITS,
			MAP_40BITS,
			MAP_48BITS,
			MAP_56BITS,
			MAP_64BITS,
			MAP_END,
			BYTES_0 = 160,
			BYTES_8BITS,
			BYTES_16BITS,
			BYTES_24BITS,
			BYTES_32BITS,
			BYTES_40BITS,
			BYTES_48BITS,
			BYTES_56BITS,
			BYTES_64BITS,
			END = 255
		};


		class Array : public Ref {
		public:
			Array();
			Array* AE_CALL copy() const;
			bool AE_CALL isContentEqual(Array* data) const;

			std::vector<SerializableObject> value;
		};


		class Map : public Ref {
		public:
			Map();
			Map(const SerializableObject& key, const SerializableObject& value);

			Map* AE_CALL copy() const;
			bool AE_CALL isContentEqual(Map* data) const;
			void AE_CALL unpack(ByteArray& ba, uint32_t size);

			std::unordered_map<SerializableObject, SerializableObject, std_unordered_hasher, std_unordered_compare> value;
		};


		class Str : public Ref {
		public:
			Str(const char* data, size_t size);
			virtual ~Str();

			char* value;
			size_t size;
		};


		template<bool Ext>
		class Bytes : public Ref {
		public:
			Bytes() :
				_data(nullptr),
				_size(0) {
				ref();
			}

			Bytes(const uint8_t* data, size_t size) :
				_size(size) {
				if constexpr (Ext) {
					_data = (uint8_t*)data;
				} else {
					if (size) {
						_data = new uint8_t[size];
						memcpy(_data, data, size);
					} else {
						_data = nullptr;
					}
				}
				ref();
			}

			virtual ~Bytes() {
				if constexpr (!Ext) {
					if (_data != nullptr) delete[] _data;
				}
			}

			inline Bytes* AE_CALL copy() const {
				return new Bytes<Ext>(_data, _size);
			}

			inline bool AE_CALL isContentEqual(const Bytes<false>& data) const {
				return isContentEqual(data._data, _size);
			}

			inline bool AE_CALL isContentEqual(const Bytes<true>& data) const {
				return isContentEqual(data._data, _size);
			}

			bool AE_CALL isContentEqual(const uint8_t* data, size_t size) const {
				if (_size == _size) {
					for (size_t i = 0; i < _size; ++i) {
						if (_data[i] != data[i]) return false;
					}

					return true;
				} else {
					return false;
				}
			}

			inline const uint8_t* AE_CALL getValue() const { return _data; }
			inline size_t AE_CALL getSize() const { return _size; }

			void AE_CALL setValue(const uint8_t* data, size_t size) {
				if (!size) {
					clear();
				} else {
					if constexpr (Ext) {
						_data = (uint8_t*)data;
						_size = size;
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
			}

			inline void AE_CALL clear() {
				if (_data != nullptr) {
					if constexpr (!Ext) delete[] _data;

					_data = nullptr;
					_size = 0;
				}
			}

			friend class Bytes<false>;
			friend class Bytes<true>;
		private:
			uint8_t* _data;
			size_t _size;
		};

		static constexpr const uint8_t VALUE_SIZE = 8;

		Type _type;
		uint8_t _value[VALUE_SIZE];

		/*
		union {
			uint64_t uint64;
			bool boolean;
			Array* array;
			Map* map;
			Bytes<true>* externalBytes;
			Bytes<false>* internalBytes;
			std::string* string;
			f32 number32;
			f64 number64;
			i8 shortString[SHORT_STRING_MAX_SIZE];
		} _value;
		*/


		bool AE_CALL _freeValue();

		template<typename T>
		inline T& AE_CALL _getValue() {
			return *(T*)_value;
		}

		template<typename T>
		inline const T& AE_CALL _getValue() const {
			return *(T*)_value;
		}

		template<typename T, Type type>
		inline void AE_CALL _set(const T& value) {
			if (_type != type) {
				_freeValue();
				_type = type;
			}
			_getValue<T>() = value;
		}

		inline void AE_CALL _setInt(const int64_t& value) {
			_set<int64_t, Type::INT>(value);
		}

		inline void AE_CALL _setUInt(const uint64_t& value) {
			_set<uint64_t, Type::UINT>(value);
		}

		inline Array* AE_CALL _getArray() {
			setArray();
			return _getValue<Array*>();
		}

		inline Map* AE_CALL _getMap() {
			setMap();
			return _getValue<Map*>();
		}

		template<bool Ext>
		inline Bytes<Ext>* AE_CALL _getBytes() {
			setBytes<Ext>();
			return _getValue<Bytes<Ext>*>();
		}

		template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
		inline bool AE_CALL _isEqual(const SerializableObject& target) const {
			switch (target._type) {
			case Type::INT:
				return _getValue<T>() == target._getValue<int64_t>();
			case Type::UINT:
				return _getValue<T>() == target._getValue<uint64_t>();
			case Type::FLOAT32:
				return _getValue<T>() == target._getValue<float32_t>();
			case Type::FLOAT64:
				return _getValue<T>() == target._getValue<float64_t>();
			default:
				return false;
			}
		}

		/*
		inline Bytes<false>* AE_CALL _getInternalBytes() {
			setBytes<false>();
			return _getValue<Bytes<false>*>();
		}

		inline Bytes<true>* AE_CALL _getExternalBytes() {
			setBytes<true>();
			return _getValue<Bytes<true>*>();
		}
		*/

		inline void AE_CALL _writeShortString(const char* s, uint32_t size) {
			memcpy(_value, s, size);
			_value[size] = 0;
		}

		inline bool AE_CALL _isContentEqual(const void* s1, size_t size1, const void* s2, size_t size2) const {
			return size1 == size2 ? memEqual(s1, s2, size1) : false;
		}

		template<typename T = std::nullptr_t, typename = std::enable_if_t<std::is_null_pointer_v<T> || std::is_base_of_v<IPackFilter, T>, T>>
		void AE_CALL _pack(const SerializableObject* parent, size_t depth, ByteArray& ba, const T& filter) const {
			switch (_type) {
			case Type::INVALID:
				ba.write<ba_t::UI8>((uint8_t)_type);
				break;
			case Type::BOOL:
				ba.write<ba_t::UI8>((uint8_t)(_getValue<bool>() ? InternalType::BOOL_TRUE : InternalType::BOOL_FALSE));
				break;
			case Type::INT:
			{
				auto v = _getValue<int64_t>();
				if (v < 0) {
					if (v == -1) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::N_INT_1);
					} else {
						_packUInt(ba, -v, (uint8_t)InternalType::N_INT_8BITS);
					}
				} else {
					if (v == 0) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::P_INT_0);
					} else if (v == 1) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::P_INT_1);
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
					ba.write<ba_t::UI8>((uint8_t)InternalType::P_INT_0);
				} else if (v == 1) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::P_INT_1);
				} else {
					_packUInt(ba, v, (uint8_t)InternalType::P_INT_8BITS);
				}

				break;
			}
			case Type::FLOAT32:
			{

				float32_t f = _getValue<float32_t>();
				if (f == 0.0f) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::FLT_0);
				} else if (f == 0.5f) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::FLT_0_5);
				} else if (f == 1.0f) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::FLT_1);
				} else {
					ba.write<ba_t::UI8>((uint8_t)_type);
					ba.write<ba_t::F32>(f);
				}

				break;
			}
			case Type::FLOAT64:
			{
				float64_t d = _getValue<float64_t>();
				if (d == 0.0) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::DBL_0);
				} else if (d == 0.5) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::DBL_0_5);
				} else if (d == 1.0) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::DBL_1);
				} else {
					ba.write<ba_t::UI8>((uint8_t)_type);
					ba.write<ba_t::F64>(d);
				}

				break;
			}
			case Type::STRING:
			{
				auto s = _getValue<Str*>();
				if (s->size) {
					ba.write<ba_t::UI8>((uint8_t)_type);
					ba.write<ba_t::STR>(s->value, s->size);
				} else {
					ba.write<ba_t::UI8>((uint8_t)InternalType::STRING_EMPTY);
				}

				break;
			}
			case Type::SHORT_STRING:
			{
				auto size = strlen((char*)_value);
				if (size == 0) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::STRING_EMPTY);
				} else {
					ba.write<ba_t::UI8>((uint8_t)Type::STRING);
					ba.write<ba_t::STR>((char*)_value, size);
				}

				break;
			}
			case Type::ARRAY:
			{
				Array* arr = _getValue<Array*>();
				auto size = arr->value.size();

				if constexpr (std::is_null_pointer_v<T>) {
					if (size == 0) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::ARRAY_0);
					} else {
						_packUInt(ba, size, (uint8_t)InternalType::ARRAY_8BITS);
						for (auto& i : arr->value) i.pack(ba);
					}
				} else {
					if (size == 0) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::ARRAY_0);
					} else {
						ba.write<ba_t::UI8>((uint8_t)InternalType::ARRAY_END);

						size_t i = 0;
						size_t d = depth + 1;
						for (auto& e : arr->value) {
							if (filter.packable(parent, depth, i++, e)) e._pack(this, d, ba, filter);
						}

						ba.write<ba_t::UI8>((uint8_t)InternalType::END);
					}
				}

				break;
			}
			case Type::MAP:
			{
				Map* map = _getValue<Map*>();
				auto size = map->value.size();

				if constexpr (std::is_null_pointer_v<T>) {
					if (size == 0) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::MAP_0);
					} else {
						_packUInt(ba, size, (uint8_t)InternalType::MAP_8BITS);
						for (auto& i : map->value) {
							i.first.pack(ba);
							i.second.pack(ba);
						}
					}
				} else {
					if (size == 0) {
						ba.write<ba_t::UI8>((uint8_t)InternalType::MAP_0);
					} else {
						ba.write<ba_t::UI8>((uint8_t)InternalType::MAP_END);

						size_t d = depth + 1;
						for (auto& i : map->value) {
							if (filter.packable(parent, depth, i.first, i.second)) {
								i.first._pack(this, d, ba, filter);
								i.second._pack(this, d, ba, filter);
							}
						}
						
						ba.write<ba_t::UI8>((uint8_t)InternalType::END);
					}
				}

				break;
			}
			case Type::BYTES:
			{
				uint32_t size = _getValue<Bytes<false>*>()->getSize();
				if (size == 0) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::BYTES_0);
					break;
				} else if (size <= BitUInt<8>::MAX) {
					_packUInt(ba, size, (uint8_t)InternalType::BYTES_8BITS);
				}

				ba.write<ba_t::BYTE>(_getValue<Bytes<false>*>()->getValue(), size);

				break;
			}
			case Type::EXT_BYTES:
			{
				uint32_t size = _getValue<Bytes<true>*>()->getSize();
				if (size == 0) {
					ba.write<ba_t::UI8>((uint8_t)InternalType::BYTES_0);
					break;
				} else {
					_packUInt(ba, size, (uint8_t)InternalType::BYTES_8BITS);
				}

				ba.write<ba_t::BYTE>(_getValue<Bytes<true>*>()->getValue(), size);

				break;
			}
			default:
				break;
			}
		}

		void AE_CALL _toJson(std::string& json) const;

		void AE_CALL _packUInt(ByteArray& ba, uint64_t val, uint8_t typeBegin) const;

		void AE_CALL _unpackBytes(ByteArray& ba, size_t size);

		//friend Hash;
	};

	AE_DEFINE_ENUM_BIT_OPERATIION(SerializableObject::ForEachOperation);

	using SO = SerializableObject;
}