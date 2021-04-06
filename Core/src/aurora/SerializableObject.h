#pragma once

#include "aurora/Intrusive.h"
#include "aurora/ByteArray.h"
#include "aurora/String.h"
#include <unordered_map>
#include <functional>

namespace aurora {
	class AE_CORE_DLL SerializableObject {
	public:
		class AE_CORE_DLL IPackFilter {
		public:
			virtual bool AE_CALL packable(const SerializableObject* parent, size_t depth, size_t index, const SerializableObject& val) const = 0;
			virtual bool AE_CALL packable(const SerializableObject* parent, size_t depth, const SerializableObject& key, const SerializableObject& val) const = 0;
		};


		enum class Type : uint8_t {
			//universal
			INVALID,
			BOOL,
			INT,
			FLOAT,
			STRING,
			ARRAY,
			MAP,
			BYTES,

			//exclusive
			UINT,

			FLOAT32 = FLOAT,
			FLOAT64 = UINT + 1,

			SHORT_STRING,
			STRING_VIEW,

			EXT_BYTES,
		};


		enum class ForEachOperation : uint8_t {
			CONTINUE,
			BREAK,
			ERASE
		};


		enum class Flag : uint8_t {
			NONE = 0,
			COPY = 0b1,
			TO_COPY = 0b10,
			DETACH_COPY = 0b100
		};


		struct AE_CORE_DLL StdUnorderedHasher {
			inline size_t AE_CALL operator()(const SerializableObject& value) const {
				switch (value._type) {
				case Type::BOOL:
					return std::hash<bool>{}(value._getValue<bool>());
				case Type::BYTES:
				{
					auto val = value._getValue<Bytes*>();
					return std::hash<void*>{}((void*)val->getValue());
				}
				case Type::EXT_BYTES:
				{
					auto& val = value._getValue<BytesView>();
					return std::hash<void*>{}((void*)val.data);
				}
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
					return std::hash<std::string_view>{}(std::string_view(str->data, str->size));
				}
				case Type::SHORT_STRING:
					return std::hash<std::string_view>{}(std::string_view((char*)value._value));
				case Type::STRING_VIEW:
				{
					auto& sv = value._getValue<StrView>();
					return std::hash<std::string_view>{}(std::string_view(sv.data, sv.size));
				}
				default:
					return 0;
				}
			}
		};


		struct AE_CORE_DLL StdUnorderedComparer {
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
		SerializableObject(const char* value, Flag flag = Flag::COPY);
		SerializableObject(const std::string& value, Flag flag = Flag::COPY);
		SerializableObject(const std::string_view& value, Flag flag = Flag::COPY);
		SerializableObject(const char8_t* value, Flag flag = Flag::COPY) : SerializableObject((const char*)value, flag) {}
		SerializableObject(const std::u8string& value, Flag flag = Flag::COPY) : SerializableObject((const std::string&)value, flag) {}
		SerializableObject(const std::u8string_view& value, Flag flag = Flag::COPY) : SerializableObject((const std::string_view&)value, flag) {}
		SerializableObject(const void* value, size_t size, Flag flag = Flag::COPY);
		SerializableObject(const ByteArray& ba, Flag flag = Flag::COPY);
		SerializableObject(const SerializableObject& value);
		SerializableObject(const SerializableObject& value, Flag flag);
		SerializableObject(SerializableObject&& value) noexcept;

		/*
		template<std::signed_integral T>
		SerializableObject::SerializableObject(T value) :
			_type(Type::INT),
			_flag(Flag::NONE) {
			_getValue<int64_t>() = value;
		}

		template<std::unsigned_integral T>
		SerializableObject::SerializableObject(T value) :
			_type(Type::UINT),
			_flag(Flag::NONE) {
			_getValue<uint64_t>() = value;
		}
		*/
		
		template<ScopedEnum T>
		SerializableObject(T value) : SerializableObject((std::underlying_type_t<T>)value) {}

		~SerializableObject();

		inline SerializableObject& AE_CALL operator=(const SerializableObject& value) {
			set(value);
			return *this;
		}
		inline SerializableObject& AE_CALL operator=(SerializableObject&& value) noexcept {
			_type = value._type;
			_flag = value._flag;
			memcpy(_value, value._value, VALUE_SIZE);
			value._type = Type::INVALID;
			value._flag = Flag::NONE;

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
		inline bool AE_CALL isString() const {
			return _type == Type::STRING || _type == Type::SHORT_STRING || _type == Type::STRING_VIEW;
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

					auto& bv = _getValue<BytesView>();
					bv.data = nullptr;
					bv.size = 0;

					_type = Type::EXT_BYTES;

					return true;
				}
			} else {
				if (_type == Type::BYTES) {
					return false;
				} else {
					_freeValue();

					_getValue<Bytes*>() = new Bytes();
					_type = Type::BYTES;

					return true;
				}
			}
		}

		bool AE_CALL toBool(bool defaultValue = false) const;

		template<Arithmetic T>
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
				return String::toNumber<T>(std::string_view(s->data, s->size));
			}
			case Type::SHORT_STRING:
				return String::toNumber<T>(std::string_view((char*)_value, strlen((char*)_value)));
			default:
				return defaultValue;
			}
		}

		template<ScopedEnum T>
		inline T AE_CALL toEnum(T defaultValue = (T)0) const {
			return (T)toNumber<std::underlying_type_t<T>>();
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
		inline T AE_CALL toValue() {
			if constexpr (Arithmetic<T>) {
				return toNumber<T>();
			} else if constexpr (std::same_as<T, std::string>) {
				return toString();
			} else if constexpr (std::same_as<T, std::string_view>) {
				return toStringView();
			} else if constexpr (std::same_as<T, bool>) {
				return toBool();
			} else if constexpr (ScopedEnum<T>) {
				return toEnum<T>();
			} else {
				return (T)0;
			}
		}

		inline void AE_CALL set(bool value) {
			_set<bool, Type::BOOL>(value);
		}

		template<std::signed_integral T>
		inline void AE_CALL set(T value) {
			_setInt(value);
		}

		template<std::unsigned_integral T>
		inline void AE_CALL set(T value) {
			_setUInt(value);
		}

		template<ScopedEnum T>
		inline void AE_CALL set(T value) {
			using I = std::underlying_type_t<T>;
			set<I>((I)value);
		}

		inline void AE_CALL set(float32_t value) {
			_set<float32_t, Type::FLOAT32>(value);
		}
		inline void AE_CALL set(const float64_t& value) {
			_set<float64_t, Type::FLOAT64>(value);
		}
		void AE_CALL set(const std::string_view& value, Flag flag = Flag::COPY);

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline void AE_CALL set(T&& value, Flag flag = Flag::COPY) {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				set(std::string_view(std::forward<T>(value)), flag);
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				set((const std::string_view&)value, flag);
			} else {
				set(std::u8string_view(std::forward<T>(value)), flag);
			}
		}

		void AE_CALL set(const void* value, size_t size, Flag flag);

		void AE_CALL set(const SerializableObject& value, Flag flag = Flag::NONE);

		SerializableObject& AE_CALL at(size_t index);
		SerializableObject AE_CALL tryAt(size_t index) const;
		SerializableObject* AE_CALL tryAtPtr(size_t index) const;
		SerializableObject& AE_CALL push();
		SerializableObject& AE_CALL push(const SerializableObject& value);
		SerializableObject AE_CALL removeAt(size_t index);
		void AE_CALL insertAt(size_t index, const SerializableObject& value);

		SerializableObject& AE_CALL get(const SerializableObject& key);
		inline SerializableObject& AE_CALL get(const std::string_view& key) {
			return get(SerializableObject(key, Flag::NONE));
		}

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline SerializableObject& AE_CALL get(T&& key) {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				return get(std::string_view(std::forward<T>(key)));
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				return get((const std::string_view&)key);
			} else {
				return get(std::u8string_view(std::forward<T>(key)));
			}
		}

		SerializableObject AE_CALL tryGet(const SerializableObject& key) const;
		inline SerializableObject AE_CALL tryGet(const std::string_view& key) const {
			return tryGet(SerializableObject(key, Flag::NONE));
		}

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline SerializableObject AE_CALL tryGet(T&& key) const {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				return tryGet(std::string_view(std::forward<T>(key)));
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				return tryGet((const std::string_view&)key);
			} else {
				return tryGet(std::u8string_view(std::forward<T>(key)));
			}
		}

		SerializableObject* AE_CALL tryGetPtr(const SerializableObject& key) const;
		inline SerializableObject* AE_CALL tryGetPtr(const std::string_view& key) const {
			return tryGetPtr(SerializableObject(key, Flag::NONE));
		}

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline SerializableObject* AE_CALL tryGetPtr(T&& key) const {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				return tryGetPtr(std::string_view(std::forward<T>(key)));
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				return tryGetPtr((const std::string_view&)key);
			} else {
				return tryGetPtr(std::u8string_view(std::forward<T>(key)));
			}
		}

		SerializableObject& AE_CALL insert(const SerializableObject& key, const SerializableObject& value);
		inline SerializableObject& AE_CALL insert(const std::string_view& key, const SerializableObject& value) {
			return insert(SerializableObject(key, Flag::TO_COPY), value);
		}

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline SerializableObject& AE_CALL insert(T&& key, const SerializableObject& value) {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				return insert(std::string_view(std::forward<T>(key)), value);
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				return insert((const std::string_view&)key, value);
			} else {
				return insert(std::u8string_view(std::forward<T>(key)), value);
			}
		}

		SerializableObject AE_CALL remove(const SerializableObject& key);
		inline SerializableObject AE_CALL remove(const std::string_view& key) {
			return remove(SerializableObject(key, Flag::NONE));
		}

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline SerializableObject AE_CALL remove(T&& key) {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				return remove(std::string_view(std::forward<T>(key)));
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				return remove((const std::string_view&)key);
			} else {
				return remove(std::u8string_view(std::forward<T>(key)));
			}
		}

		bool AE_CALL has(const SerializableObject& key) const;
		inline bool AE_CALL has(const std::string_view& key) const {
			return has(SerializableObject(key, Flag::NONE));
		}

		template<typename T>
		requires (!std::same_as<std::remove_cvref_t<T>, std::string_view>) && ConvertibleString8Data<std::remove_cvref_t<T>>
		inline bool AE_CALL has(T&& key) const {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<T>>) {
				return has(std::string_view(std::forward<T>(key)));
			} else if constexpr (std::same_as<std::remove_cvref_t<T>, std::u8string_view>) {
				return has((const std::string_view&)key);
			} else {
				return has(std::u8string_view(std::forward<T>(key)));
			}
		}

		template<InvocableAnyOfResult<std::tuple<void, bool>, const SerializableObject&, const SerializableObject&> Fn>
		void AE_CALL forEach(Fn&& fn) const {
			if (_type == Type::ARRAY) {
				if (Array* arr = _getValue<Array*>(); arr) {
					SerializableObject idx;
					for (size_t i = 0, n = arr->value.size(); i < n; ++i) {
						idx.set(i);
						if constexpr (std::same_as<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, void>) {
							fn(idx, arr->value[i]);
						} else {
							if (!fn(idx, arr->value[i])) break;
						}
					}
				}
			} else if (_type == Type::MAP) {
				if (Map* map = _getValue<Map*>(); map) {
					for (auto& itr : map->value) {
						if constexpr (std::same_as<std::invoke_result_t<Fn, const SerializableObject&, const SerializableObject&>, void>) {
							fn(itr.first, itr.second);
						} else {
							if (!fn(itr.first, itr.second)) break;
						}
					}
				}
			}
		}
		
		template<InvocableResult<ForEachOperation, const SerializableObject&, SerializableObject&> Fn>
		void AE_CALL forEach(Fn&& fn) {
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

		template<NullPointerOrDerivedFrom<IPackFilter> T = std::nullptr_t>
		inline void AE_CALL pack(ByteArray& ba, const T& filter = nullptr) const {
			_pack(nullptr, 0, ba, filter);
		}
		void AE_CALL unpack(ByteArray& ba, Flag flag = Flag::COPY);

		bool AE_CALL isEqual(const SerializableObject& target) const;
		bool AE_CALL isContentEqual(const SerializableObject& target) const;

		inline bool operator==(const SerializableObject& right) const {
			return isEqual(right);
		}

		inline bool operator!=(const SerializableObject& right) const {
			return !isEqual(right);
		}

	private:
		static constexpr uint8_t BOOL_FALSE = ((uint8_t)Type::INVALID << 5) | 1;
		static constexpr uint8_t BOOL_TRUE = ((uint8_t)Type::INVALID << 5) | 2;
		static constexpr uint8_t END = ((uint8_t)Type::INVALID << 5) | 3;
		static constexpr uint8_t VAL4_MAX = 0b1111;
		static constexpr uint8_t VAL4_BITS8 = VAL4_MAX - 8;
		static constexpr uint8_t VAL4_BITS64 = VAL4_MAX;
		static constexpr uint8_t VAL5_MAX = 0b11111;
		static constexpr uint8_t VAL5_BITS8 = VAL5_MAX - 8;
		static constexpr uint8_t VAL5_BITS64 = VAL5_MAX;


		class Array {
			AE_REF_OBJECT(Array)
		public:
			Array();
			Array* AE_CALL copy(Flag flag) const;
			bool AE_CALL isContentEqual(Array* data) const;

			std::vector<SerializableObject> value;
		};


		class Map {
			AE_REF_OBJECT(Map)
		public:
			Map();
			Map* AE_CALL copy(Flag flag) const;
			bool AE_CALL isContentEqual(Map* data) const;
			void AE_CALL unpack(ByteArray& ba, size_t size, Flag flag);

			std::unordered_map<SerializableObject, SerializableObject, StdUnorderedHasher, StdUnorderedComparer> value;
		};


		class Str {
			AE_REF_OBJECT(Str)
		public:
			Str(const char* data, size_t size);
			~Str();

			char* data;
			size_t size;
		};


		struct StrView {
			const char* data;
			size_t size;
		};


		struct BytesView {
			const uint8_t* data;
			size_t size;

			inline bool operator==(const BytesView& right) const {
				return data == right.data && size == right.size;
			}
		};


		class Bytes {
			AE_REF_OBJECT(Bytes)
		public:
			Bytes();
			Bytes(const uint8_t* data, size_t size);
			~Bytes();

			inline Bytes* AE_CALL copy() const {
				return new Bytes(_data, _size);
			}

			inline const uint8_t* AE_CALL getValue() const { return _data; }
			inline size_t AE_CALL getSize() const { return _size; }

			void AE_CALL setValue(const uint8_t* data, size_t size);

			inline void AE_CALL clear() {
				if (_data != nullptr) {
					delete[] _data;

					_data = nullptr;
					_size = 0;
				}
			}

		private:
			uint8_t* _data;
			size_t _size;
		};

		static constexpr uint8_t VALUE_SIZE = 16;

		Type _type;
		Flag _flag;
		uint8_t _value[VALUE_SIZE];

		void AE_CALL _freeValue();

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

		template<Arithmetic T>
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

		inline void AE_CALL _writeShortString(const char* s, size_t size) {
			memcpy(_value, s, size);
			_value[size] = 0;
		}

		inline void AE_CALL _writeStringView(const char* s, size_t size) {
			auto& sv = _getValue<StrView>();
			sv.data = s;
			sv.size = size;
		}

		inline bool AE_CALL _isContentEqual(const void* s1, size_t size1, const void* s2, size_t size2) const {
			return size1 == size2 ? !memcmp(s1, s2, size1) : false;
		}

		template<NullPointerOrDerivedFrom<IPackFilter> T = std::nullptr_t>
		void AE_CALL _pack(const SerializableObject* parent, size_t depth, ByteArray& ba, const T& filter) const {
			switch (_type) {
			case Type::INVALID:
				ba.write<uint8_t>((uint8_t)_type);
				break;
			case Type::BOOL:
				ba.write<uint8_t>((uint8_t)(_getValue<bool>() ? BOOL_TRUE : BOOL_FALSE));
				break;
			case Type::INT:
			{
				auto v = _getValue<int64_t>();
				if (v < 0) {
					if (-v >= VAL4_BITS8) {
						_packUInt(ba, -v, ((uint8_t)Type::INT << 5) | 0b10000, VAL4_BITS8);
					} else {
						ba.write<uint8_t>(((uint8_t)Type::INT << 5) | 0b10000 | -v);
					}
				} else {
					if (v >= VAL4_BITS8) {
						_packUInt(ba, v, (uint8_t)Type::INT << 5, VAL4_BITS8);
					} else {
						ba.write<uint8_t>(((uint8_t)Type::INT << 5) | v);
					}
				}

				break;
			}
			case Type::UINT:
			{
				auto v = _getValue<uint64_t>();
				if (v >= VAL4_BITS8) {
					_packUInt(ba, v, (uint8_t)Type::INT << 5, VAL4_BITS8);
				} else {
					ba.write<uint8_t>(((uint8_t)Type::INT << 5) | v);
				}

				break;
			}
			case Type::FLOAT32:
			{
				auto v = _getValue<float32_t>();
				if (v == 0.0f) {
					ba.write<uint8_t>((uint8_t)Type::FLOAT << 5);
				} else {
					auto tpos = ba.getPosition();
					ba.write<ba_vt::PADDING>(1);
					uint8_t t = (uint8_t)Type::FLOAT << 5;
					auto p = (uint8_t*)&v;
					for (size_t i = 0; i < 4; ++i) {
						if (p[i] != 0) {
							t |= 0b1 << i;
							ba.write<uint8_t>(p[i]);
						}
					}
					ba.getSource()[tpos] = t;
				}

				break;
			}
			case Type::FLOAT64:
			{
				auto v = _getValue<float64_t>();
				if (v == 0.0) {
					ba.write<uint8_t>(((uint8_t)Type::FLOAT << 5) | 0b10000);
				} else {
					auto tpos = ba.getPosition();
					ba.write<ba_vt::PADDING>(1);
					uint8_t t = ((uint8_t)Type::FLOAT << 5) | 0b10000;
					auto p = (uint8_t*)&v;
					auto wp = (uint16_t*)&v;
					for (size_t i = 0; i < 4; ++i) {
						if (wp[i] != 0) {
							auto ii = i << 1;
							t |= 0b1 << i;
							ba.write<uint8_t>(p[ii]);
							ba.write<uint8_t>(p[++ii]);
						}
					}
					ba.getSource()[tpos] = t;
				}

				break;
			}
			case Type::STRING:
			{
				auto s = _getValue<Str*>();
				_packString(ba, s->data, s->size);

				break;
			}
			case Type::SHORT_STRING:
				_packString(ba, (char*)_value, strlen((char*)_value));
				break;
			case Type::STRING_VIEW:
			{
				auto& sv = _getValue<StrView>();
				_packString(ba, sv.data, sv.size);

				break;
			}
			case Type::ARRAY:
			{
				auto arr = _getValue<Array*>();
				auto size = arr->value.size();

				if constexpr (NullPointer<T>) {
					if (size < VAL5_MAX) {
						ba.write<uint8_t>(((uint8_t)Type::ARRAY << 5) | size);
						for (auto& i : arr->value) i.pack(ba);
					} else {
						ba.write<uint8_t>(((uint8_t)Type::ARRAY << 5) | VAL5_MAX);
						for (auto& i : arr->value) i.pack(ba);
						ba.write<uint8_t>(END);
					}
				} else {
					size_t d = depth + 1;
					std::vector<size_t> indices;
					for (size_t i = 0, n = arr->value.size(); i < n;  ++i) {
						if (filter.packable(parent, depth, i, arr->value[i])) indices.emplace_back(i);
					}

					if (indices.size() < VAL5_MAX) {
						ba.write<uint8_t>(((uint8_t)Type::ARRAY << 5) | size);
						for (auto& i : indices) arr->value[i]._pack(this, d, ba, filter);
					} else {
						ba.write<uint8_t>(((uint8_t)Type::ARRAY << 5) | VAL5_MAX);
						for (auto& i : arr->value) arr->value[i]._pack(this, d, ba, filter);
						ba.write<uint8_t>(END);
					}
				}

				break;
			}
			case Type::MAP:
			{
				auto map = _getValue<Map*>();
				auto size = map->value.size();

				if constexpr (NullPointer<T>) {
					if (size < VAL5_MAX) {
						ba.write<uint8_t>(((uint8_t)Type::MAP << 5) | size);
						for (auto& i : map->value) {
							i.first.pack(ba);
							i.second.pack(ba);
						}
					} else {
						ba.write<uint8_t>(((uint8_t)Type::MAP << 5) | VAL5_MAX);
						for (auto& i : map->value) {
							i.first.pack(ba);
							i.second.pack(ba);
						}
						ba.write<uint8_t>(END);
					}
				} else {
					size_t d = depth + 1;
					std::vector<SerializableObject*> data;
					for (auto& i : map->value) {
						if (filter.packable(parent, depth, i.first, i.second)) {
							data.emplace_back(&i.first);
							data.emplace_back(&i.second);
						}
					}

					size_t size = data.size() >> 1;
					if (size < VAL5_MAX) {
						ba.write<uint8_t>(((uint8_t)Type::ARRAY << 5) | size);
						for (auto& i : data) i->_pack(this, d, ba, filter);
					} else {
						ba.write<uint8_t>(((uint8_t)Type::ARRAY << 5) | VAL5_MAX);
						for (auto& i : data) i->_pack(this, d, ba, filter);
						ba.write<uint8_t>(END);
					}
				}

				break;
			}
			case Type::BYTES:
			{
				auto val = _getValue<Bytes*>();
				_packBytes(ba, val->getValue(), val->getSize());

				break;
			}
			case Type::EXT_BYTES:
			{
				auto& bv = _getValue<BytesView>();
				_packBytes(ba, bv.data, bv.size);

				break;
			}
			default:
				break;
			}
		}

		void AE_CALL _toJson(std::string& json) const;

		void AE_CALL _packUInt(ByteArray& ba, uint64_t val, uint8_t typeBegin, uint8_t typeEnd) const;
		void AE_CALL _packString(ByteArray& ba, const char* data, size_t size) const;
		void AE_CALL _packBytes(ByteArray& ba, const uint8_t* data, size_t size) const;

		void AE_CALL _unpackBytes(ByteArray& ba, size_t size, Flag flag);
	};

	using SO = SerializableObject;
}