#pragma once

#include "srk/Bit.h"
#include "srk/StringUtility.h"
#include <algorithm>

namespace srk {
	class SRK_CORE_DLL ByteArray {
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
		ByteArray& SRK_CALL operator=(const ByteArray&) = delete;
		ByteArray(ByteArray&& bytes) noexcept;
		ByteArray(size_t capacity, size_t length = 0);
		ByteArray(void* bytes, size_t size, Usage usage = Usage::SHARED);
		ByteArray(void* bytes, size_t capacity, size_t length, Usage usage = Usage::SHARED);
		ByteArray& SRK_CALL operator=(ByteArray&& value) noexcept;
		~ByteArray();

		inline SRK_CALL operator bool() const {
			return _length > 0;
		}

		inline void SRK_CALL dispose(bool free = true) {
			_dispose(free);
			clear();
		}

		inline void SRK_CALL clear() {
			_position = 0;
			_length = 0;
		}
		inline void SRK_CALL detach() {
			if (_usage == Usage::SHARED) _resize(_capacity);
		}

		inline bool SRK_CALL isValid() const {
			return _data != nullptr;
		}

		inline std::endian SRK_CALL getEndian() const {
			return _endian;
		}
		inline void SRK_CALL setEndian(std::endian endian) {
			_endian = endian;
			_needReverse = _endian != std::endian::native;
		}

		inline uint8_t* SRK_CALL getSource() {
			return _data;
		}
		inline const uint8_t* SRK_CALL getSource() const {
			return _data;
		}

		inline uint8_t* SRK_CALL getCurrentSource() {
			return _data + _position;
		}
		inline const uint8_t* SRK_CALL getCurrentSource() const {
			return _data + _position;
		}

		inline ByteArray SRK_CALL slice(size_t start, size_t length, Usage usage = Usage::SHARED) const {
			if (start >= _length) {
				ByteArray ba(nullptr, 0, usage);
				ba.setEndian(_endian);
				return ba;
			} else {
				ByteArray ba(_data + start, std::min<size_t>(length, _length - start), usage);
				ba.setEndian(_endian);
				return ba;
			}
		}
		inline ByteArray SRK_CALL slice(size_t length, Usage usage = Usage::SHARED) const {
			ByteArray ba(_data + _position, std::min<size_t>(length, _length - _position), usage);
			ba.setEndian(_endian);
			return ba;
		}
		inline ByteArray SRK_CALL slice(Usage usage = Usage::SHARED) const {
			ByteArray ba(_data + _position, _length - _position, usage);
			ba.setEndian(_endian);
			return ba;
		}

		inline size_t SRK_CALL getCapacity() const {
			return _capacity;
		}
		inline void SRK_CALL setCapacity(size_t capacity) {
			if (_capacity != capacity) {
				_resize(capacity);
				if (_length > _capacity) {
					_length = _capacity;
					if (_position > _length) _position = _length;
				}
			}
		}

		inline size_t SRK_CALL getLength() const {
			return _length;
		}
		inline void SRK_CALL setLength(size_t len) {
			if (len > _capacity) _dilatation(len);

			_length = len;
			if (_position > _length) _position = _length;
		}

		inline size_t SRK_CALL getPosition() const {
			return _position;
		}
		inline void SRK_CALL setPosition(size_t pos) {
			_position = pos > _length ? _length : pos;
		}

		inline void SRK_CALL skip(size_t size) {
			setPosition(_position + size);
		}

		inline ByteArray& SRK_CALL seekBegin() {
			_position = 0;
			return *this;
		}
		inline ByteArray& SRK_CALL seekEnd() {
			_position = _length;
			return *this;
		}

		inline size_t SRK_CALL getBytesAvailable() const {
			return _length - _position;
		}

		void SRK_CALL popFront(size_t len);
		void SRK_CALL popBack(size_t len);
		void SRK_CALL insert(size_t len);

		template<Arithmetic T, bool AlignedAccess = false>
		inline T SRK_CALL read() {
			return _read<T, AlignedAccess>();
		}

		template<ValueType T>
		requires (T == ValueType::BOOL)
		inline bool SRK_CALL read() {
			return _read<bool, false>();
		}

		template<ValueType T>
		requires (T == ValueType::I8)
		inline int8_t SRK_CALL read() {
			return _read<int8_t, false>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::I16)
		inline int16_t SRK_CALL read() {
			return _read<int16_t, AlignedAccess>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::I32)
		inline int32_t SRK_CALL read() {
			return _read<int32_t, AlignedAccess>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::I64)
		inline int64_t SRK_CALL read() {
			return _read<int64_t, AlignedAccess>();
		}

		template<ValueType T>
		requires (T == ValueType::UI8)
		inline uint8_t SRK_CALL read() {
			return _read<uint8_t, false>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::UI16)
		inline uint16_t SRK_CALL read() {
			return _read<uint16_t, AlignedAccess>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::UI32)
		inline uint32_t SRK_CALL read() {
			return _read<uint32_t, AlignedAccess>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::UI64)
		inline uint64_t SRK_CALL read() {
			return _read<uint64_t, AlignedAccess>();
		}

		template<ValueType T>
		requires EqualAnyOf<T, ValueType::D_UI64, ValueType::RD_UI64>
		inline uint64_t SRK_CALL read() {
			if constexpr (T == ValueType::D_UI64) {
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

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::IX)
		int64_t SRK_CALL read(uint8_t numBytes) {
			switch (numBytes) {
			case 1:
				return read<ba_vt::I8>();
			case 2:
				return read<ba_vt::UI16, AlignedAccess>();
			case 4:
				return read<ba_vt::I32, AlignedAccess>();
			case 8:
				return read<ba_vt::I64, AlignedAccess>();
			case 3:
				return _readIX<3, AlignedAccess>();
			case 5:
				return _readIX<5, AlignedAccess>();
			case 6:
				return _readIX<6, AlignedAccess>();
			case 7:
				return _readIX<7, AlignedAccess>();
			default:
				return 0;
			}
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::UIX)
		uint64_t SRK_CALL read(uint8_t numBytes) {
			switch (numBytes) {
			case 1:
				return read<ba_vt::UI8>();
			case 2:
				return read<ba_vt::UI16, AlignedAccess>();
			case 4:
				return read<ba_vt::UI32, AlignedAccess>();
			case 8:
				return read<ba_vt::UI64, AlignedAccess>();
			case 3:
				return _readUIX<3, AlignedAccess>();
			case 5:
				return _readUIX<5, AlignedAccess>();
			case 6:
				return _readUIX<6, AlignedAccess>();
			case 7:
				return _readUIX<7, AlignedAccess>();
			default:
				return 0;
			}
		}

		template<ValueType T>
		requires (T == ValueType::TWO_I12)
		inline std::tuple<int16_t, int16_t> SRK_CALL read() {
			auto [v1, v2] = read<ValueType::TWO_UI12>();
			return std::make_tuple<int16_t, int16_t>(v1 > Bit::intMax<12>() ? v1 - Bit::uintMax<12>() - 1 : v1, v2 > Bit::intMax<12>() ? v2 - Bit::uintMax<12>() - 1 : v2);
		}

		template<ValueType T>
		requires (T == ValueType::TWO_UI12)
		inline std::tuple<uint16_t, uint16_t> SRK_CALL read() {
			auto v1 = read<ValueType::UI8>();
			auto v2 = read<ValueType::UI8>();
			auto v3 = read<ValueType::UI8>();

			return std::make_tuple<uint16_t, uint16_t>((v1 << 4) | ((v2 >> 4) & 0xF), ((v2 & 0xF) << 8) | v3);
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::F32)
		inline float32_t SRK_CALL read() {
			return _read<float32_t, AlignedAccess>();
		}

		template<ValueType T, bool AlignedAccess = false>
		requires (T == ValueType::F64)
		inline float64_t SRK_CALL read() {
			return _read<float64_t, AlignedAccess>();
		}

		template<StringData T, bool CheckEndMark = true, bool CheckBOM = false>
		inline T SRK_CALL read(size_t size = (std::numeric_limits<size_t>::max)()) {
			return _read<T, CheckEndMark, CheckBOM>(size);
		}

		template<ValueType T, bool CheckEndMark = true, bool CheckBOM = false>
		requires (T == ValueType::STR)
		inline std::string SRK_CALL read(size_t size = (std::numeric_limits<size_t>::max)()) {
			return _read<std::string, CheckEndMark, CheckBOM>(size);
		}

		template<ValueType T, bool CheckEndMark = true, bool CheckBOM = false>
		requires (T == ValueType::STR_V)
		inline std::string_view SRK_CALL read(size_t size = (std::numeric_limits<size_t>::max)()) {
			return _read<std::string_view, CheckEndMark, CheckBOM>(size);
		}

		template<ValueType T, bool CheckEndMark = true, bool CheckBOM = false>
		requires (T == ValueType::STR)
		//begin, size, position
		inline std::tuple<size_t, size_t, size_t> SRK_CALL read(size_t begin, size_t size) const {
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

		template<ValueType T>
		requires (T == ValueType::STR)
		//begin, size
		inline std::tuple<size_t, size_t> SRK_CALL read(size_t size) {
			auto [begin, s, pos] = read<T>(_position, size);
			_position = pos;
			return std::make_tuple(begin, s);
		}

		template<ValueType T>
		requires (T == ValueType::BYTE)
		size_t SRK_CALL read(void* bytes, size_t length = (std::numeric_limits<size_t>::max)()) {
			size_t len = _length - _position;
			if (length > len) length = len;

			memmove(bytes, _data + _position, length);
			_position += length;

			return length;
		}

		template<std::same_as<std::byte> T>
		inline void SRK_CALL read(void* bytes, size_t length = (std::numeric_limits<size_t>::max)()) {
			read<ValueType::BYTE>(bytes, length);
		}

		template<ValueType T>
		requires (T == ValueType::BYTE)
		size_t SRK_CALL read(ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)()) {
			size_t len = _length - _position;
			if (length > len) length = len;

			auto pos = ba.getPosition();
			ba.setPosition(offset);
			ba.write<T>(_data + _position, length);
			ba.setPosition(pos);
			_position += length;

			return length;
		}

		template<std::same_as<std::byte> T>
		inline void SRK_CALL read(ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)()) {
			read<ValueType::BYTE>(ba, offset, length);
		}

		template<Arithmetic T>
		inline void SRK_CALL write(T value) {
			_write<T>(value);
		}

		template<ValueType T>
		requires (T == ValueType::BOOL)
		inline void SRK_CALL write(bool value) {
			_write<bool>(value);
		}

		template<ValueType T>
		requires (T == ValueType::I8)
		inline void SRK_CALL write(int8_t value) {
			_write<int8_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::I16)
		inline void SRK_CALL write(int16_t value) {
			_write<int16_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::I32)
		inline void SRK_CALL write(int32_t value) {
			_write<int32_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::I64)
		inline void SRK_CALL write(int64_t value) {
			_write<int64_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::UI8)
		inline void SRK_CALL write(uint8_t value) {
			_write<uint8_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::UI16)
		inline void SRK_CALL write(uint16_t value) {
			_write<uint16_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::UI32)
		inline void SRK_CALL write(uint32_t value) {
			_write<uint32_t>(value);
		}

		template<ValueType T>
		requires EqualAnyOf<T, ValueType::UI64, ValueType::D_UI64, ValueType::RD_UI64, ValueType::PADDING>
		inline void SRK_CALL write(uint64_t value) {
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

		template<ValueType T>
		requires (T == ValueType::IX)
		void SRK_CALL write(int64_t value, uint8_t numBytes) {
			switch (numBytes) {
			case 1:
				write<ValueType::I8>(value);
				break;
			case 2:
				write<ValueType::I16>(value);
				break;
			case 3:
			{
				if (value < 0) value = Bit::uintMax<24>() + 1 + value;
				_write(&value, 3);

				break;
			}
			case 4:
				write<ValueType::I32>(value);
				break;
			case 5:
			{
				if (value < 0) value = Bit::uintMax<40>() + 1 + value;
				_write(&value, 5);

				break;
			}
			case 6:
			{
				if (value < 0) value = Bit::uintMax<48>() + 1 + value;
				_write(&value, 6);

				break;
			}
			case 7:
			{
				if (value < 0) value = Bit::uintMax<56>() + 1 + value;
				_write(&value, 7);

				break;
			}
			case 8:
				write<ValueType::I64>(value);
				break;
			default:
				break;
			}
		}

		template<ValueType T>
		requires (T == ValueType::UIX)
		inline void SRK_CALL write(uint64_t value, uint8_t numBytes) {
			if (numBytes > 0 && numBytes < 9) _write(&value, numBytes);
		}

		template<ValueType T>
		requires (T == ValueType::TWO_I12)
		inline void SRK_CALL write(int16_t value1, int16_t value2) {
			write<ValueType::TWO_UI12>(value1 < 0 ? Bit::uintMax<12>() + 1 + value1 : value1, value2 < 0 ? Bit::uintMax<12>() + 1 + value2 : value2);
		}

		template<ValueType T>
		requires (T == ValueType::TWO_UI12)
		inline void SRK_CALL write(uint16_t value1, uint16_t value2) {
			write<ValueType::UI8>((value1 >> 4) & 0xFF);
			write<ValueType::UI8>(((value1 & 0xF) << 4) | ((value2 >> 8) & 0xF));
			write<ValueType::UI8>(value2 & 0xFF);
		}

		template<ValueType T>
		requires (T == ValueType::F32)
		inline void SRK_CALL write(float32_t value) {
			_write<float32_t>(value);
		}

		template<ValueType T>
		requires (T == ValueType::F64)
		inline void SRK_CALL write(float64_t value) {
			_write<float64_t>(value);
		}

		template<ValueType T, typename V>
		requires (T == ValueType::STR || T == ValueType::BYTE) && ConvertibleString8Data<std::remove_cvref_t<V>>
		inline void SRK_CALL write(V&& value) {
			if constexpr (T == ValueType::STR) {
				write(value);
			} else {
				if constexpr (String8Data<std::remove_cvref_t<V>>) {
					write<T>(value.data(), value.size());
				} else {
					write<T>(ConvertToString8ViewType<std::remove_cvref_t<V>>(std::forward<V>(value)));
				}
			}
		}

		template<typename T>
		requires ConvertibleString8Data<std::remove_cvref_t<T>>
		inline void SRK_CALL write(T&& value) {
			if constexpr (String8Data<std::remove_cvref_t<T>>) {
				auto size = value.size();

				_checkLength(size + 1);

				memmove(_data + _position, value.data(), size);
				_position += size;
				_data[_position++] = '\0';
			} else {
				write(ConvertToString8ViewType<std::remove_cvref_t<T>>(std::forward<T>(value)));
			}
		}

		template<ValueType T, SameAnyOf<char, char8_t> V>
		requires (T == ValueType::STR)
		inline void SRK_CALL write(const V* value, size_t size) {
			write(ConvertToString8ViewType<V*>(value, size));
		}

		template<ValueType T, bool Reverse = false>
		requires (T == ValueType::BYTE)
		void SRK_CALL write(const void* bytes, size_t length) {
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

		template<std::same_as<std::byte> T>
		inline void SRK_CALL write(const void* bytes, size_t length) {
			write<ValueType::BYTE>(bytes, length);
		}

		template<ValueType T>
		requires (T == ValueType::BYTE)
		size_t SRK_CALL write(const ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)()) {
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

		template<std::same_as<std::byte> T>
		inline void SRK_CALL write(const ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)()) {
			write<ValueType::BYTE>(ba, offset, length);
		}

		inline static bool SRK_CALL equal(const ByteArray& data1, const ByteArray& data2) {
			return data1.getLength() == data2.getLength() ? equal(data1.getSource(), data2.getSource(), data1.getLength()) : false;
		}
		inline static bool SRK_CALL equal(const void* data1, const void* data2, size_t len) {
			return memcmp(data1, data2, len) == 0;
		}

	private:
		Usage _usage;
		bool _needReverse;
		std::endian _endian;
		uint8_t* _data;
		size_t _position;
		size_t _length;
		size_t _capacity;

		inline void SRK_CALL _dispose(bool free = true) {
			if (_data) {
				if (free && _usage != Usage::SHARED) delete[] _data;
				_data = nullptr;
			}
		}

		template<size_t Bytes, bool AlignedAccess>
		inline int_t<Bytes * 8> SRK_CALL _readIX() {
			int_t<Bytes * 8> v = _readUIX<Bytes, AlignedAccess>();
			return v > Bit::intMax<Bytes * 8>() ? v - Bit::uintMax<Bytes * 8>() - 1 : v;
		}

		template<size_t Bytes, bool AlignedAccess>
		uint_t<Bytes * 8> SRK_CALL _readUIX() {
			if (_position + Bytes > _length) {
				_position = _length;
				return 0;
			} else {
				if (_needReverse) {
					auto v = Bit::byteswap<Bytes, AlignedAccess>(&_data[_position]);
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

		template<Arithmetic T, bool AlignedAccess>
		T SRK_CALL _read() {
			if constexpr (sizeof(T) == 1) {
				if (_position < _length) {
					return *(T*)&_data[_position++];
				} else {
					_position = _length;
					return (T)0;
				}
			} else {
				constexpr auto TYPE_BYTES = sizeof(T);
				if (_position + TYPE_BYTES > _length) {
					_position = _length;
					return (T)0;
				}

				if (_needReverse) {
					auto v = Bit::byteswap<TYPE_BYTES, AlignedAccess>(&_data[_position]);
					_position += TYPE_BYTES;
					if constexpr (std::integral<T>) {
						return v;
					} else {
						return *(T*)&v;
					}
				} else {
					T v;
					if constexpr (AlignedAccess) {
						memcpy(&v, &_data[_position], sizeof(T));
					} else {
						v = *(T*)&_data[_position];
					}

					_position += TYPE_BYTES;
					return v;
				}
			}
		}

		template<StringData T, bool CheckEndMark, bool CheckBOM>
		inline T SRK_CALL _read(size_t size) {
			auto [begin, num, pos] = read<ValueType::STR, CheckEndMark, CheckBOM>(_position, size);
			_position = pos;
			return T((char*)_data + begin, num);
		}

		inline void SRK_CALL _write(const void* data, size_t len) {
			auto p = (const uint8_t*)data;

			_checkLength(len);

			if (_needReverse) {
				for (size_t i = 0, j = len; i < len; ++i) _data[_position + --j] = p[i];
			} else {
				memmove(_data + _position, p, len);
			}

			_position += len;
		}

		template<Arithmetic T>
		inline void SRK_CALL _write(const T& value) {
			_write(&value, sizeof(T));
		}

		void SRK_CALL _resize(size_t len);
		inline void SRK_CALL _dilatation(size_t size) {
			_resize(size > 1 ? size + (size >> 1) : size + 1);
		}
		inline void SRK_CALL _checkLength(size_t len) {
			len += _position;
			if (len > _length) {
				_length = len;
				if (_length > _capacity) _dilatation(len);
			}
		}

		inline uint8_t SRK_CALL _bomOffset(size_t pos) const {
			if (pos + 3 > _length) return 0;
			return _data[pos] == 0xEF && _data[pos + 1] == 0xBB && _data[pos + 2] == 0xBF ? 3 : 0;
		}
	};

	using ba_vt = ByteArray::ValueType;
}