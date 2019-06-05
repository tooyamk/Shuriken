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
		enum class ValueType : ui8 {
			INVALID,

			BOOL,
			INTEGET,

			FLOAT,
			DOUBLE,

			STRING,
			ARRAY,
			MAP,

			BYTES,
			EXT_BYTES,

			SHORT_STRING
		};


		enum class ForEachOperation : ui8 {
			CONTINUE,
			BREAK,
			ERASE
		};


		struct HashKey {
			std::size_t operator()(const SerializableObject& value) const {
				std::size_t key = 0;

				switch (value._type) {
				case ValueType::BOOL:
					key = std::hash<bool>{}(value._value.boolean);
					break;
				case ValueType::BYTES:
					key = std::hash<Bytes<false>*>{}(value._value.internalBytes);
					break;
				case ValueType::EXT_BYTES:
					return std::hash<Bytes<true>*>{}(value._value.externalBytes);
					break;
				case ValueType::FLOAT:
					return std::hash<float>{}(value._value.number32);
					break;
				case ValueType::DOUBLE:
					return std::hash<double>{}(value._value.number64);
					break;
				case ValueType::INTEGET:
					return std::hash<long long>{}(value._value.uint64);
					break;
				case ValueType::ARRAY:
					return std::hash<Array*>{}(value._value.array);
					break;
				case ValueType::MAP:
					return std::hash<Map*>{}(value._value.map);
					break;
				case ValueType::STRING:
					return std::hash<std::string>{}(*value._value.string);
					break;
				case ValueType::SHORT_STRING:
					return std::hash<std::string>{}(value._value.shortString);
					break;
				default:
					break;
				}

				constexpr const i32 n = sizeof(std::size_t) / 8;
				auto keys = (ui8*)&key;
				auto idx = NATIVE_ENDIAN == Endian::LITTLE ? n - 1 : 0;
				keys[idx] = ((ui32)value._type) | (keys[idx] & 0xF);

				return key;
			}
		};


		struct HashCompare {
			bool operator()(const SerializableObject& value1, const SerializableObject& value2) const {
				return value1 == value2;
			}
		};


		SerializableObject();
		SerializableObject(ValueType value);
		SerializableObject(bool value);
		SerializableObject(i8 value);
		SerializableObject(ui8 value);
		SerializableObject(i16 value);
		SerializableObject(ui16 value);
		SerializableObject(i32 value);
		SerializableObject(ui32 value);
		SerializableObject(const i64& value);
		SerializableObject(const ui64& value);
		SerializableObject(f32 value);
		SerializableObject(const f64& value);
		SerializableObject(const i8* value);
		SerializableObject(const std::string& value);
		SerializableObject(const std::string_view& value);
		SerializableObject(const i8* value, ui32 size, bool copy);
		SerializableObject(ByteArray& ba, bool copy);
		SerializableObject(const SerializableObject& key, const SerializableObject& value);
		SerializableObject& operator=(const SerializableObject& value);
		SerializableObject& operator=(SerializableObject&& value);
		SerializableObject(const SerializableObject& value);
		SerializableObject(const SerializableObject& value, bool copy);
		SerializableObject(SerializableObject&& value);
		~SerializableObject();

		ValueType AE_CALL getType() const;
		bool AE_CALL isValid() const;

		ui32 getSize() const;
		void clearValues();

		bool AE_CALL setInvalid();
		bool AE_CALL setArray();
		bool AE_CALL setMap();

		template<bool Ext>
		bool AE_CALL setBytes() {
			if constexpr (Ext) {
				if (_type == ValueType::EXT_BYTES) {
					return false;
				} else {
					_freeValue();

					_value.externalBytes = new Bytes<Ext>();
					_type = ValueType::EXT_BYTES;

					return true;
				}
			} else {
				if (_type == ValueType::BYTES) {
					return false;
				} else {
					_freeValue();

					_value.internalBytes = new Bytes<Ext>();
					_type = ValueType::BYTES;

					return true;
				}
			}
		}

		bool AE_CALL toBool(bool defaultValue = false) const;

		template<typename T,
		typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
		T AE_CALL toNumber(T defaultValue = 0) const {
			switch (_type) {
			case ValueType::BOOL:
				return _value.boolean ? 1 : 0;
				break;
			case ValueType::INTEGET:
				return _value.uint64;
				break;
			case ValueType::FLOAT:
				return _value.number32;
				break;
			case ValueType::DOUBLE:
				return _value.number64;
				break;
			case ValueType::STRING:
				return String::toNumber<T>(*_value.string);
				break;
			case ValueType::SHORT_STRING:
				return String::toNumber<T>(_value.shortString, strlen(_value.shortString));
				break;
			default:
				return defaultValue;
				break;
			}
		}

		std::string AE_CALL toString(const std::string& defaultValue = "") const;
		const i8* AE_CALL toBytes() const;

		void AE_CALL set(bool value);
		void AE_CALL set(i8 value);
		void AE_CALL set(ui8 value);
		void AE_CALL set(i16 value);
		void AE_CALL set(ui16 value);
		void AE_CALL set(i32 value);
		void AE_CALL set(ui32 value);
		void AE_CALL set(const i64& value);
		void AE_CALL set(const ui64& value);
		void AE_CALL set(f32 value);
		void AE_CALL set(const f64& value);
		void AE_CALL set(const i8* value);
		void AE_CALL set(const std::string& value);
		void AE_CALL set(const std::string_view& value);
		void AE_CALL set(const i8* value, ui32 size, bool copy);

		void AE_CALL set(const SerializableObject& value, bool copy = false);

		SerializableObject& AE_CALL at(ui32 index);
		SerializableObject AE_CALL tryAt(ui32 index) const;
		void AE_CALL push(const SerializableObject& value);
		SerializableObject AE_CALL removeAt(ui32 index);
		void AE_CALL insertAt(ui32 index, const SerializableObject& value);

		SerializableObject& AE_CALL get(const SerializableObject& key);
		SerializableObject AE_CALL tryGet(const SerializableObject& key) const;
		SerializableObject& AE_CALL insert(const SerializableObject& key, const SerializableObject& value);
		SerializableObject AE_CALL remove(const SerializableObject& key);
		bool AE_CALL has(const SerializableObject& key) const;

		void AE_CALL forEach(const std::function<ForEachOperation(const SerializableObject& key, SerializableObject& value)>& callback);

		void AE_CALL pack(ByteArray& ba) const;
		void AE_CALL unpack(ByteArray& ba);

		bool AE_CALL isContentEqual(const SerializableObject& sv) const;

		bool operator==(const SerializableObject& right) const;

	private:
		enum class InternalValueType : ui8 {
			UNVALID = (ui8)ValueType::INVALID,

			BOOL = (ui8)ValueType::BOOL,
			INTEGET = (ui8)ValueType::INTEGET,

			FLOAT = (ui8)ValueType::FLOAT,
			DOUBLE = (ui8)ValueType::DOUBLE,

			STRING = (ui8)ValueType::STRING,
			ARRAY = (ui8)ValueType::ARRAY,
			MAP = (ui8)ValueType::MAP,

			BYTES = (ui8)ValueType::BYTES,
			EXT_BYTES = (ui8)ValueType::EXT_BYTES,

			SHORT_STRING = (ui8)ValueType::SHORT_STRING,

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
			U_INT_0 = 40,
			U_INT_1,
			U_INT_8BITS,
			U_INT_16BITS,
			U_INT_24BITS,
			U_INT_32BITS,
			U_INT_40BITS,
			U_INT_48BITS,
			U_INT_56BITS,
			U_INT_64BITS,
			FLOAT_0 = 60,
			FLOAT_0_5,
			FLOAT_1,
			N_FLOAT_INT_8BITS,
			N_FLOAT_INT_16BITS,
			N_FLOAT_INT_24BITS,
			U_FLOAT_INT_8BITS,
			U_FLOAT_INT_16BITS,
			U_FLOAT_INT_24BITS,
			DOUBLE_0 = 80,
			DOUBLE_0_5,
			DOUBLE_1,
			STRING_EMPTY = 100,
			ARRAY_0 = 120,
			ARRAY_8BITS,
			ARRAY_16BITS,
			ARRAY_24BITS,
			ARRAY_32BITS,
			MAP_0 = 140,
			MAP_8BITS,
			MAP_16BITS,
			MAP_24BITS,
			MAP_32BITS,
			BYTES_0 = 160,
			BYTES_8BITS,
			BYTES_16BITS,
			BYTES_24BITS,
			BYTES_32BITS
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
			void AE_CALL unpack(ByteArray& ba, ui32 size);

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

			Bytes(const i8* data, ui32 size) :
				_size(size) {
				if constexpr (Ext) {
					_data = (i8*)data;
				} else {
					_data = new i8[size];
					if (data) memcpy(_data, data, size);
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

			bool AE_CALL isContentEqual(const i8* data, ui32 size) const {
				if (_size == _size) {
					for (ui32 i = 0; i < _size; ++i) {
						if (_data[i] != data[i]) return false;
					}

					return true;
				} else {
					return false;
				}
			}

			inline const i8* AE_CALL getValue() const { return _data; }
			inline ui32 AE_CALL getSize() const { return _size; }

			void AE_CALL setValue(const i8* data, ui32 size) {
				if (size == 0) {
					clear();
				} else {
					if constexpr (Ext) {
						_data = (i8*)data;
						_size = size;
					} else {
						if (_data == nullptr) {
							_size = size;
							_data = new i8[_size];
							if (data) memcpy(_data, data, _size);
						} else if (_size == size) {
							if (data) memcpy(_data, data, _size);
						} else {
							delete[] _data;
							_size = size;
							_data = new i8[_size];
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
			i8* _data;
			ui32 _size;
		};

		static constexpr const ui8 SHORT_STRING_MAX_SIZE = (std::max<ui8>(sizeof(ui64), sizeof(uintptr_t)) << 1) - sizeof(ValueType);

		ValueType _type;

		union {
			ui64 uint64;
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

		bool AE_CALL _freeValue();

		inline void AE_CALL _setInteger(const unsigned long long& value) {
			if (_type != ValueType::INTEGET) {
				_freeValue();
				_type = ValueType::INTEGET;
			}
			_value.uint64 = value;
		}

		inline Array* AE_CALL _getArray() {
			setArray();
			return _value.array;
		}

		inline Map* AE_CALL _getMap() {
			setMap();
			return _value.map;
		}

		inline Bytes<false>* AE_CALL _getInternalBytes() {
			setBytes<false>();
			return _value.internalBytes;
		}

		inline Bytes<true>* AE_CALL _getExternalBytes() {
			setBytes<true>();
			return _value.externalBytes;
		}

		inline void AE_CALL _writeShortString(const i8* s, ui32 size) {
			memcpy(_value.shortString, s, size);
			_value.shortString[size] = 0;
		}

		inline bool AE_CALL _isContentEqual(const i8* s1, ui32 size1, const i8* s2, ui32 size2) const {
			if (size1 == size2) {
				for (ui32 i = 0; i < size1; ++i) {
					if (s1[i] != s2[i]) return false;
				}
				return true;
			} else {
				return false;
			}
		}

		inline bool AE_CALL _isContentEqual(const std::string& s1, const i8* s2) const {
			return _isContentEqual(s1.c_str(), s1.size(), s2, strlen(s2));
		}

		inline bool AE_CALL _isContentEqual(const i8* s1, const i8* s2) const {
			return _isContentEqual(s1, strlen(s1), s2, strlen(s2));
		}

		void AE_CALL _unpackArray(ByteArray& ba, ui32 size);
		void AE_CALL _unpackMap(ByteArray& ba, ui32 size);
		void AE_CALL _unpackBytes(ByteArray& ba, ui32 size);

		//friend Hash;
	};

	AE_DEFINE_ENUM_BIT_OPERATIION(SerializableObject::ForEachOperation);

	using SO = SerializableObject;
}