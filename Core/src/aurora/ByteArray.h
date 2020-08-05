#pragma once

#include "aurora/Global.h"
#include <algorithm>
#include <string_view>

namespace aurora {
	class AE_CORE_DLL ByteArray {
	public:
		enum class Usage : uint8_t {
			SHARED,
			COPY,
			EXCLUSIVE
		};


		enum class ValueType : uint8_t {
			BOOL,
			I8,
			I16,
			I32,
			I64,
			UI8,
			UI16,
			UI32,
			UI64,
			D_UI64,
			RD_UI64,
			IX,
			UIX,
			TWO_I12,
			TWO_UI12,
			F32,
			F64,
			STR,
			STR_V,
			BYTE,
			PADDING
		};

	private:
		using ba_vt = ValueType;

	public:


		//enum class CompressionAlgorithm : ui8 {
		//	ZLIB
		//};

		ByteArray();
		ByteArray(const ByteArray&) = delete;
		ByteArray& AE_CALL operator=(const ByteArray&) = delete;
		ByteArray(ByteArray&& bytes) noexcept;
		ByteArray(size_t capacity, size_t length = 0);
		ByteArray(void* bytes, size_t size, Usage usage = Usage::SHARED);
		ByteArray(void* bytes, size_t capacity, size_t length, Usage usage = Usage::SHARED);
		ByteArray& AE_CALL operator=(ByteArray&& value) noexcept;
		~ByteArray();

		inline AE_CALL operator bool() const {
			return _length > 0;
		}

		inline void AE_CALL dispose(bool free = true) {
			_dispose(free);
			clear();
		}

		inline void AE_CALL clear() {
			_position = 0;
			_length = 0;
		}
		inline void AE_CALL detach() {
			if (_usage == Usage::SHARED) _resize(_capacity);
		}

		inline bool AE_CALL isValid() const {
			return _data != nullptr;
		}

		inline std::endian AE_CALL getEndian() const {
			return _endian;
		}
		inline void AE_CALL setEndian(std::endian endian) {
			_endian = endian;
			_needReverse = _endian != std::endian::native;
		}

		inline uint8_t* AE_CALL getSource() {
			return _data;
		}
		inline const uint8_t* AE_CALL getSource() const {
			return _data;
		}

		inline uint8_t* AE_CALL getCurrentSource() {
			return _data + _position;
		}
		inline const uint8_t* AE_CALL getCurrentSource() const {
			return _data + _position;
		}

		inline ByteArray AE_CALL slice(size_t start, size_t length, Usage usage = Usage::SHARED) const {
			if (start >= _length) {
				ByteArray ba(nullptr, 0, usage);
				ba.setEndian(_endian);
				return std::move(ba);
			} else {
				ByteArray ba(_data + start, std::min<size_t>(length, _length - start), usage);
				ba.setEndian(_endian);
				return std::move(ba);
			}
		}
		inline ByteArray AE_CALL slice(size_t length, Usage usage = Usage::SHARED) const {
			ByteArray ba(_data + _position, std::min<size_t>(length, _length - _position), usage);
			ba.setEndian(_endian);
			return std::move(ba);
		}
		inline ByteArray AE_CALL slice(Usage usage = Usage::SHARED) const {
			ByteArray ba(_data + _position, _length - _position, usage);
			ba.setEndian(_endian);
			return std::move(ba);
		}

		inline size_t AE_CALL getCapacity() const {
			return _capacity;
		}
		inline void AE_CALL setCapacity(size_t capacity) {
			if (_capacity != capacity) {
				_resize(capacity);
				if (_length > _capacity) {
					_length = _capacity;
					if (_position > _length) _position = _length;
				}
			}
		}

		inline size_t AE_CALL getLength() const {
			return _length;
		}
		inline void AE_CALL setLength(size_t len) {
			if (len > _capacity) _dilatation(len);

			_length = len;
			if (_position > _length) _position = _length;
		}

		inline size_t AE_CALL getPosition() const {
			return _position;
		}
		inline void AE_CALL setPosition(size_t pos) {
			_position = pos > _length ? _length : pos;
		}

		inline void AE_CALL skip(size_t size) {
			setPosition(_position + size);
		}

		inline void AE_CALL seekBegin() {
			_position = 0;
		}
		inline void AE_CALL seekEnd() {
			_position = _length;
		}

		inline size_t AE_CALL getBytesAvailable() const {
			return _length - _position;
		}

		void AE_CALL popFront(size_t len);
		void AE_CALL popBack(size_t len);
		void AE_CALL insert(size_t len);

		template<typename T, typename = typename std::enable_if_t<
			std::is_same_v<T, bool> ||
			std::is_same_v<T, int8_t> ||
			std::is_same_v<T, uint8_t> ||
			std::is_same_v<T, int16_t> ||
			std::is_same_v<T, uint16_t> ||
			std::is_same_v<T, int32_t> ||
			std::is_same_v<T, uint32_t> ||
			std::is_same_v<T, int64_t> ||
			std::is_same_v<T, uint64_t> ||
			std::is_same_v<T, float32_t> ||
			std::is_same_v<T, float64_t>, bool>>
		inline T AE_CALL read() {
			return _read<T>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::BOOL, bool>>
		inline bool AE_CALL read() {
			return _read<bool>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I8, bool>>
		inline int8_t AE_CALL read() {
			return _read<int8_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I16, bool>>
		inline int16_t AE_CALL read() {
			return _read<int16_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I32, bool>>
		inline int32_t AE_CALL read() {
			return _read<int32_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I64, bool>>
		inline int64_t AE_CALL read() {
			return _read<int64_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI8, bool>>
		inline uint8_t AE_CALL read() {
			return _read<uint8_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI16, bool>>
		inline uint16_t AE_CALL read() {
			return _read<uint16_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI32, bool>>
		inline uint32_t AE_CALL read() {
			return _read<uint32_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI64 || T == ValueType::D_UI64 || T == ValueType::RD_UI64, bool>>
		inline uint64_t AE_CALL read() {
			if constexpr (T == ValueType::UI64) {
				return _read<uint64_t>();
			} else if constexpr (T == ValueType::D_UI64) {
				uint64_t rst = 0;
				uint32_t bits = 0;
				while (_position < _length) {
					uint64_t val = _data[_position++];
					auto hasNext = (val & 0x80) != 0;
					val &= 0x7F;
					val <<= bits;
					rst |= val;

					if (hasNext) {
						bits += 7;
					} else {
						break;
					}
				};

				return rst;
			} else {
				uint64_t rst = 0;
				uint32_t bits = 0;
				while (_position > 0) {
					uint64_t val = _data[--_position];
					auto hasNext = (val & 0x80) != 0;
					val &= 0x7F;
					val <<= bits;
					rst |= val;

					if (hasNext) {
						bits += 7;
					} else {
						break;
					}
				};

				return rst;
			}
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::IX, bool>>
		int64_t AE_CALL read(uint8_t numBytes) {
			switch (numBytes) {
			case 1:
				return read<ba_vt::I8>();
			case 2:
				return read<ba_vt::UI16>();
			case 4:
				return read<ba_vt::I32>();
			case 8:
				return read<ba_vt::I64>();
			case 3:
				return _readIX<3>();
			case 5:
				return _readIX<5>();
			case 6:
				return _readIX<6>();
			case 7:
				return _readIX<7>();
			default:
				return 0;
			}
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UIX, bool>>
		uint64_t AE_CALL read(uint8_t numBytes) {
			switch (numBytes) {
			case 1:
				return read<ba_vt::UI8>();
			case 2:
				return read<ba_vt::UI16>();
			case 4:
				return read<ba_vt::UI32>();
			case 8:
				return read<ba_vt::UI64>();
			case 3:
				return _readUIX<3>();
			case 5:
				return _readUIX<5>();
			case 6:
				return _readUIX<6>();
			case 7:
				return _readUIX<7>();
			default:
				return 0;
			}
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::TWO_I12, bool>>
		inline std::tuple<int16_t, int16_t> AE_CALL read() {
			auto [v1, v2] = read<ValueType::TWO_UI12>();
			return std::make_tuple<int16_t, int16_t>(v1 > BitInt<12>::MAX ? v1 - BitUInt<12>::MAX - 1 : v1, v2 > BitInt<12>::MAX ? v2 - BitUInt<12>::MAX - 1 : v2);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::TWO_UI12, bool>>
		inline std::tuple<uint16_t, uint16_t> AE_CALL read() {
			auto v1 = read<ValueType::UI8>();
			auto v2 = read<ValueType::UI8>();
			auto v3 = read<ValueType::UI8>();

			return std::make_tuple<uint16_t, uint16_t>((v1 << 4) | ((v2 >> 4) & 0xF), ((v2 & 0xF) << 8) | v3);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::F32, bool>>
		inline float32_t AE_CALL read() {
			return _read<float32_t>();
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::F64, bool>>
		inline float64_t AE_CALL read() {
			return _read<float64_t>();
		}

		template<typename T, bool CheckEndMark = true, bool CheckBOM = false, typename = string_data_t<T>>
		inline T AE_CALL read(size_t size = (std::numeric_limits<size_t>::max)()) {
			return _read<T, CheckEndMark, CheckBOM>(size);
		}

		template<ValueType T, bool CheckEndMark = true, bool CheckBOM = false, typename = typename std::enable_if_t<T == ValueType::STR, bool>>
		inline std::string AE_CALL read(size_t size = (std::numeric_limits<size_t>::max)()) {
			return _read<std::string, CheckEndMark, CheckBOM>(size);
		}

		template<ValueType T, bool CheckEndMark = true, bool CheckBOM = false, typename = typename std::enable_if_t<T == ValueType::STR_V, bool>>
		inline std::string_view AE_CALL read(size_t size = (std::numeric_limits<size_t>::max)()) {
			return _read<std::string_view, CheckEndMark, CheckBOM>(size);
		}

		template<ValueType T, bool CheckEndMark = true, bool CheckBOM = false, typename = typename std::enable_if_t<T == ValueType::STR, bool>>
		//begin, size, position
		inline std::tuple<size_t, size_t, size_t> AE_CALL read(size_t begin, size_t size) const {
			if constexpr (CheckBOM) {
				begin += _bomOffset(begin);
			}
			if (_length <= begin) return std::make_tuple(begin, 0, begin);

			if (auto len = _length - begin; size > len) size = len;
			if (!size) return std::make_tuple(begin, 0, begin);

			if constexpr (CheckEndMark) {
				for (size_t i = begin, n = begin + size; i < n; ++i) {
					if (!_data[i]) {
						auto s = i - begin;
						return std::make_tuple(begin, s, begin + s + 1);
					}
				}
			}

			return std::make_tuple(begin, size, begin + size);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::STR, bool>>
		//begin, size
		inline std::tuple<size_t, size_t> AE_CALL read(size_t size) {
			auto [begin, s, pos] = read<T>(_position, size);
			_position = pos;
			return std::make_tuple(begin, s);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::BYTE, bool>>
		size_t AE_CALL read(void* bytes, size_t length = (std::numeric_limits<size_t>::max)()) {
			size_t len = _length - _position;
			if (length > len) length = len;

			memmove(bytes, _data + _position, length);
			_position += length;

			return length;
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::BYTE, bool>>
		size_t AE_CALL read(ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)()) {
			size_t len = _length - _position;
			if (length > len) length = len;

			auto pos = ba.getPosition();
			ba.setPosition(offset);
			ba.write<T>(_data + _position, length);
			ba.setPosition(pos);
			_position += length;

			return length;
		}

		template<typename T, typename = typename std::enable_if_t<
			std::is_same_v<T, bool> ||
			std::is_same_v<T, int8_t> ||
			std::is_same_v<T, uint8_t> ||
			std::is_same_v<T, int16_t> ||
			std::is_same_v<T, uint16_t> ||
			std::is_same_v<T, int32_t> ||
			std::is_same_v<T, uint32_t> ||
			std::is_same_v<T, int64_t> ||
			std::is_same_v<T, uint64_t> ||
			std::is_same_v<T, float32_t> ||
			std::is_same_v<T, float64_t>, bool>>
		inline void AE_CALL write(T value) {
			_write<T>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::BOOL, bool>>
		inline void AE_CALL write(bool value) {
			_write<bool>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I8, bool>>
		inline void AE_CALL write(int8_t value) {
			_write<int8_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I16, bool>>
		inline void AE_CALL write(int16_t value) {
			_write<int16_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I32, bool>>
		inline void AE_CALL write(int32_t value) {
			_write<int32_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::I64, bool>>
		inline void AE_CALL write(int64_t value) {
			_write<int64_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI8, bool>>
		inline void AE_CALL write(uint8_t value) {
			_write<uint8_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI16, bool>>
		inline void AE_CALL write(uint16_t value) {
			_write<uint16_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI32, bool>>
		inline void AE_CALL write(uint32_t value) {
			_write<uint32_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UI64 || T == ValueType::D_UI64 || T == ValueType::RD_UI64 || T == ValueType::PADDING, bool>>
		inline void AE_CALL write(uint64_t value) {
			if constexpr (T == ValueType::UI64) {
				_write<uint64_t>(value);
			} else if constexpr (T == ValueType::D_UI64 || T == ValueType::RD_UI64) {
				value &= 0xFFFFFFFFFFFFFFFULL;
				uint8_t i = 0;
				uint8_t vals[8];
				do {
					uint8_t val = value & 0x7F;
					value >>= 7;
					if (value) {
						val |= 0x80;
						vals[i++] = val;
					} else {
						vals[i++] = val;
						break;
					}
				} while (true);

				write<ValueType::BYTE, T == ValueType::RD_UI64>(vals, i);
			} else {
				_checkLength(value);
				_position += value;
			}
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::IX, bool>>
		void AE_CALL write(int64_t value, uint8_t numBytes) {
			switch (numBytes) {
			case 1:
				write<ValueType::I8>(value);
				break;
			case 2:
				write<ValueType::I16>(value);
				break;
			case 3:
			{
				if (value < 0) value = BitUInt<24>::MAX + 1 + value;
				_write((uint8_t*)&value, 3);

				break;
			}
			case 4:
				write<ValueType::I32>(value);
				break;
			case 5:
			{
				if (value < 0) value = BitUInt<40>::MAX + 1 + value;
				_write((uint8_t*)&value, 5);

				break;
			}
			case 6:
			{
				if (value < 0) value = BitUInt<48>::MAX + 1 + value;
				_write((uint8_t*)&value, 6);

				break;
			}
			case 7:
			{
				if (value < 0) value = BitUInt<56>::MAX + 1 + value;
				_write((uint8_t*)&value, 7);

				break;
			}
			case 8:
				write<ValueType::I64>(value);
				break;
			default:
				break;
			}
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::UIX, bool>>
		inline void AE_CALL write(uint64_t value, uint8_t numBytes) {
			if (numBytes > 0 && numBytes < 9) _write((uint8_t*)&value, numBytes);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::TWO_I12, bool>>
		inline void AE_CALL write(int16_t value1, int16_t value2) {
			write<ValueType::TWO_UI12>(value1 < 0 ? BitUInt<12>::MAX + 1 + value1 : value1, value2 < 0 ? BitUInt<12>::MAX + 1 + value2 : value2);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::TWO_UI12, bool>>
		inline void AE_CALL write(uint16_t value1, uint16_t value2) {
			write<ValueType::UI8>((value1 >> 4) & 0xFF);
			write<ValueType::UI8>(((value1 & 0xF) << 4) | ((value2 >> 8) & 0xF));
			write<ValueType::UI8>(value2 & 0xFF);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::F32, bool>>
		inline void AE_CALL write(float32_t value) {
			_write<float32_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::F64, bool>>
		inline void AE_CALL write(float64_t value) {
			_write<float64_t>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::STR, bool>>
		inline void AE_CALL write(const std::string& value) {
			write<std::string>(value);
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::STR, bool>>
		inline void AE_CALL write(const std::string_view& value) {
			write<std::string_view>(value);
		}

		template<typename T, typename = typename std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>, bool>>
		void AE_CALL write(const T& value) {
			auto size = value.size();

			_checkLength(size + 1);

			memmove(_data + _position, value.data(), size);
			_position += size;
			_data[_position++] = '\0';
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::STR, bool>>
		inline void AE_CALL write(const char* value, size_t size) {
			write<std::string_view>(std::string_view(value, size));
		}

		template<ValueType T, bool Reverse = false, typename = typename std::enable_if_t<T == ValueType::BYTE, bool>>
		inline void AE_CALL write(const void* bytes, size_t length) {
			if (length > 0) {
				_checkLength(length);

				if constexpr (Reverse) {
					auto src = (const uint8_t*)bytes + length - 1;
					auto dst = _data + _position;
					for (size_t i = 0; i < length; ++i) {
						*dst = *src;
						++dst;
						--src;
					}
				} else {
					memmove(_data + _position, bytes, length);
				}

				_position += length;
			}
		}

		template<ValueType T, typename V, typename = typename std::enable_if_t<T == ValueType::BYTE && is_string_data_v<V>, bool>>
		inline void AE_CALL write(const V& str) {
			write<T>(str.data(), str.size());
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::BYTE, bool>>
		inline void AE_CALL write(const char* str) {
			write<T>(str, strlen(str));
		}

		template<ValueType T, typename = typename std::enable_if_t<T == ValueType::BYTE, bool>>
		size_t AE_CALL write(const ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)()) {
			if (!length) return 0;
			auto len = ba.getLength();
			if (len <= offset) return 0;

			len -= offset;
			if (length > len) length = len;
			auto baBytes = ba.getSource();

			_checkLength(length);

			memmove(_data + _position, baBytes + offset, length);
			_position += length;
			return length;
		}

		inline static bool isEqual(const ByteArray& data1, const ByteArray& data2) {
			return data1.getLength() == data2.getLength() ? isEqual(data1.getSource(), data2.getSource(), data1.getLength()) : false;
		}
		static bool isEqual(const uint8_t* data1, const uint8_t* data2, size_t len);

	private:
		Usage _usage;
		bool _needReverse;
		std::endian _endian;
		uint8_t* _data;
		size_t _position;
		size_t _length;
		size_t _capacity;

		inline void AE_CALL _dispose(bool free = true) {
			if (_data) {
				if (free && _usage != Usage::SHARED) delete[] _data;
				_data = nullptr;
			}
		}

		template<size_t Bytes>
		inline int_t<Bytes * 8> AE_CALL _readIX() {
			int_t<Bytes * 8> v = _readUIX<Bytes>();
			return v > BitInt<Bytes * 8>::MAX ? v - BitUInt<Bytes * 8>::MAX - 1 : v;
		}

		template<size_t Bytes>
		uint_t<Bytes * 8> AE_CALL _readUIX() {
			if (_position + Bytes > _length) {
				_position = _length;
				return 0;
			} else {
				if (_needReverse) {
					auto v = byteswap<Bytes>(&_data[_position]);
					_position += 3;
					return v;
				} else {
					uint_t<Bytes * 8> v = _data[_position];
					for (size_t i = 1; i < Bytes; ++i) v |= (uint_t<Bytes * 8>)_data[_position + i] << (i * 8);
					_position += Bytes;
					return v;
				}
			}
		}

		template<typename T, typename = typename std::enable_if_t<
			std::is_same_v<T, bool> ||
			std::is_same_v<T, int8_t> ||
			std::is_same_v<T, uint8_t> ||
			std::is_same_v<T, int16_t> ||
			std::is_same_v<T, uint16_t> ||
			std::is_same_v<T, int32_t> ||
			std::is_same_v<T, uint32_t> ||
			std::is_same_v<T, int64_t> ||
			std::is_same_v<T, uint64_t> ||
			std::is_same_v<T, float32_t> ||
			std::is_same_v<T, float64_t>, T>>
		T AE_CALL _read() {
			if constexpr (sizeof(T) == 1) {
				if (_position < _length) {
					return *(T*)&_data[_position++];
				} else {
					_position = _length;
					return (T)0;
				}
			} else {
				constexpr uint32_t TYPE_BYTES = sizeof(T);
				if (_position + TYPE_BYTES > _length) {
					_position = _length;
					return (T)0;
				}

				if (_needReverse) {
					auto v = byteswap<TYPE_BYTES>(&_data[_position]);
					_position += TYPE_BYTES;
					if constexpr (std::is_integral_v<T>) {
						return v;
					} else {
						return *(T*)&v;
					}
				} else {
					T v = *(T*)&_data[_position];
					_position += TYPE_BYTES;
					return v;
				}
			}
		}

		template<typename T, bool CheckEndMark, bool CheckBOM, typename = string_data_t<T>>
		inline T AE_CALL _read(size_t size) {
			auto [begin, num, pos] = read<ValueType::STR, CheckEndMark, CheckBOM>(_position, size);
			_position = pos;
			return std::move(T((char*)_data + begin, num));
		}

		inline void AE_CALL _write(const uint8_t* p, uint32_t len) {
			_checkLength(len);

			if (_needReverse) {
				for (int32_t i = len - 1; i >= 0; --i) _data[_position++] = p[i];
			} else {
				memmove(_data + _position, p, len);
				_position += len;
			}
		}

		template<typename T, typename = typename std::enable_if_t<
			std::is_same_v<T, bool> ||
			std::is_same_v<T, int8_t> ||
			std::is_same_v<T, uint8_t> ||
			std::is_same_v<T, int16_t> ||
			std::is_same_v<T, uint16_t> ||
			std::is_same_v<T, int32_t> ||
			std::is_same_v<T, uint32_t> ||
			std::is_same_v<T, int64_t> ||
			std::is_same_v<T, uint64_t> ||
			std::is_same_v<T, float32_t> ||
			std::is_same_v<T, float64_t>, T>>
		inline void AE_CALL _write(const T& value) {
			_write((uint8_t*)&value, sizeof(T));
		}

		void AE_CALL _resize(size_t len);
		inline void AE_CALL _dilatation(size_t size) {
			_resize(size > 1 ? size + (size >> 1) : size + 1);
		}
		inline void AE_CALL _checkLength(size_t len) {
			len += _position;
			if (len > _length) {
				_length = len;
				if (_length > _capacity) _dilatation(len);
			}
		}

		inline uint8_t AE_CALL _bomOffset(size_t pos) const {
			if (pos + 3 > _length) return 0;
			return (uint8_t)_data[pos] == 0xEF && (uint8_t)_data[pos + 1] == 0xBB && (uint8_t)_data[pos + 2] == 0xBF ? 3 : 0;
		}
	};

	using ba_vt = ByteArray::ValueType;
}