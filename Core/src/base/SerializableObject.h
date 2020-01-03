#pragma once

#include "base/Ref.h"
#include "base/ByteArray.h"
#include "base/String.h"
#include <unordered_map>
#include <functional>

namespace aurora {
	class SerializationValueMapElement;

	class AE_DLL SerializableObject {
	public:
		enum class Type : uint8_t {
			INVALID,

			BOOL,

			INT,
			UINT,

			FLOAT,
			DOUBLE,

			STRING,
			ARRAY,
			MAP,

			BYTES,
			EXT_BYTES,

			SHORT_STRING
		};


		enum class ForEachOperation : uint8_t {
			CONTINUE,
			BREAK,
			ERASE
		};


		struct HashKey {
			size_t operator()(const SerializableObject& value) const {
				switch (value._type) {
				case Type::BOOL:
					return std::hash<bool>{}(value._getValue<bool>());
				case Type::BYTES:
					return std::hash<Bytes<false>*>{}(value._getValue<Bytes<false>*>());
				case Type::EXT_BYTES:
					return std::hash<Bytes<true>*>{}(value._getValue<Bytes<true>*>());
				case Type::FLOAT:
					return std::hash<f32>{}(value._getValue<f32>());
				case Type::DOUBLE:
					return std::hash<f64>{}(value._getValue<f64>());
				case Type::INT:
				case Type::UINT:
					return std::hash<uint64_t>{}(value._getValue<uint64_t>());
				case Type::ARRAY:
					return std::hash<Array*>{}(value._getValue<Array*>());
				case Type::MAP:
					return std::hash<Map*>{}(value._getValue<Map*>());
				case Type::STRING:
					return std::hash<std::string>{}(*value._getValue<std::string*>());
				case Type::SHORT_STRING:
					return std::hash<std::string>{}((char*)value._value);
				default:
					return 0;
				}
			}
		};


		struct HashCompare {
			bool operator()(const SerializableObject& value1, const SerializableObject& value2) const {
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
		SerializableObject(f32 value);
		SerializableObject(const f64& value);
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

		template<typename T,
		typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
		T AE_CALL toNumber(T defaultValue = 0) const {
			switch (_type) {
			case Type::BOOL:
				return _getValue<bool>()? 1 : 0;
			case Type::INT:
				return _getValue<int64_t>();
			case Type::UINT:
				return _getValue<uint64_t>();
			case Type::FLOAT:
				return _getValue<f32>();
			case Type::DOUBLE:
				return _getValue<f64>();
			case Type::STRING:
				return String::toNumber<T>(*_getValue<std::string*>());
			case Type::SHORT_STRING:
				return String::toNumber<T>((char*)_value, strlen((char*)_value));
			default:
				return defaultValue;
			}
		}

		std::string AE_CALL toString(const std::string& defaultValue = "") const;
		const uint8_t* AE_CALL toBytes() const;

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
		inline void AE_CALL set(f32 value) {
			_set<f32, Type::FLOAT>(value);
		}
		inline void AE_CALL set(const f64& value) {
			_set<f64, Type::DOUBLE>(value);
		}
		void AE_CALL set(const char* value);
		void AE_CALL set(const std::string& value);
		void AE_CALL set(const std::string_view& value);
		void AE_CALL set(const uint8_t* value, size_t size, bool copy);

		void AE_CALL set(const SerializableObject& value, bool copy = false);

		SerializableObject& AE_CALL at(size_t index);
		SerializableObject AE_CALL tryAt(size_t index) const;
		void AE_CALL push(const SerializableObject& value);
		SerializableObject AE_CALL removeAt(size_t index);
		void AE_CALL insertAt(size_t index, const SerializableObject& value);

		SerializableObject& AE_CALL get(const SerializableObject& key);
		SerializableObject AE_CALL tryGet(const SerializableObject& key) const;
		SerializableObject& AE_CALL insert(const SerializableObject& key, const SerializableObject& value);
		SerializableObject AE_CALL remove(const SerializableObject& key);
		bool AE_CALL has(const SerializableObject& key) const;

		void AE_CALL forEach(const std::function<void(const SerializableObject& key, const SerializableObject& value)>& callback) const;
		void AE_CALL forEach(const std::function<bool(const SerializableObject& key, const SerializableObject& value)>& callback) const;
		void AE_CALL forEach(const std::function<ForEachOperation(const SerializableObject& key, SerializableObject& value)>& callback);

		void AE_CALL pack(ByteArray& ba) const;
		void AE_CALL unpack(ByteArray& ba);

		bool AE_CALL isEqual(const SerializableObject& target) const;
		bool AE_CALL isContentEqual(const SerializableObject& target) const;

		bool operator==(const SerializableObject& right) const;

	private:
		enum class InternalType : uint8_t {
			UNVALID = (uint8_t)Type::INVALID,

			BOOL = (uint8_t)Type::BOOL,
			INT = (uint8_t)Type::INT,
			UINT = (uint8_t)Type::UINT,

			FLOAT = (uint8_t)Type::FLOAT,
			DOUBLE = (uint8_t)Type::DOUBLE,

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
			MAP_0 = 140,
			MAP_8BITS,
			MAP_16BITS,
			MAP_24BITS,
			MAP_32BITS,
			MAP_40BITS,
			MAP_48BITS,
			MAP_56BITS,
			MAP_64BITS,
			BYTES_0 = 160,
			BYTES_8BITS,
			BYTES_16BITS,
			BYTES_24BITS,
			BYTES_32BITS,
			BYTES_40BITS,
			BYTES_48BITS,
			BYTES_56BITS,
			BYTES_64BITS
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

			std::unordered_map<SerializableObject, SerializableObject, HashKey, HashCompare> value;
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

		template<typename T,
		typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
		inline bool AE_CALL _isEqual(const SerializableObject& target) const {
			switch (target._type) {
			case Type::INT:
				return _getValue<T>() == target._getValue<int64_t>();
			case Type::UINT:
				return _getValue<T>() == target._getValue<uint64_t>();
			case Type::FLOAT:
				return _getValue<T>() == target._getValue<f32>();
			case Type::DOUBLE:
				return _getValue<T>() == target._getValue<f64>();
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

		inline bool AE_CALL _isContentEqual(const uint8_t* s1, size_t size1, const uint8_t* s2, size_t size2) const {
			if (size1 == size2) {
				for (size_t i = 0; i < size1; ++i) {
					if (s1[i] != s2[i]) return false;
				}
				return true;
			} else {
				return false;
			}
		}

		inline bool AE_CALL _isContentEqual(const std::string& s1, const char* s2) const {
			return _isContentEqual((uint8_t*)s1.data(), s1.size(), (uint8_t*)s2, strlen(s2));
		}

		inline bool AE_CALL _isContentEqual(const char* s1, const char* s2) const {
			return _isContentEqual((uint8_t*)s1, strlen(s1), (uint8_t*)s2, strlen(s2));
		}

		void AE_CALL _packUInt(ByteArray& ba, uint64_t val, uint8_t typeBegin) const;

		void AE_CALL _unpackArray(ByteArray& ba, size_t size);
		void AE_CALL _unpackMap(ByteArray& ba, size_t size);
		void AE_CALL _unpackBytes(ByteArray& ba, size_t size);

		//friend Hash;
	};

	AE_DEFINE_ENUM_BIT_OPERATIION(SerializableObject::ForEachOperation);

	using SO = SerializableObject;
}