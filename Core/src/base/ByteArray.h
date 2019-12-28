#pragma once

#include "base/Global.h"
#include <algorithm>
#include <string_view>

namespace aurora {
	class AE_DLL ByteArray {
	public:
		enum class Usage : uint8_t {
			SHARED,
			COPY,
			EXCLUSIVE
		};


		//enum class CompressionAlgorithm : ui8 {
		//	ZLIB
		//};


		ByteArray(const ByteArray&) = delete;
		ByteArray& AE_CALL operator=(const ByteArray&) = delete;
		ByteArray(ByteArray&& bytes) noexcept;
		ByteArray(size_t capacity = 0, size_t length = 0);
		ByteArray(uint8_t* bytes, size_t size, Usage usage = Usage::SHARED);
		ByteArray(uint8_t* bytes, size_t capacity, size_t length, Usage usage = Usage::SHARED);
		ByteArray& AE_CALL operator=(ByteArray&& value) noexcept;
		~ByteArray();

		inline AE_CALL operator bool() const;

		void AE_CALL dispose(bool free = true);

		inline void AE_CALL clear();
		inline void AE_CALL detach();

		inline bool AE_CALL isValid() const;

		inline std::endian AE_CALL getEndian() const;
		inline void AE_CALL setEndian(std::endian endian);

		inline uint8_t* AE_CALL getSource();
		inline const uint8_t* AE_CALL getSource() const;

		inline ByteArray AE_CALL slice(size_t start, size_t length, Usage usage = Usage::SHARED) const;
		inline ByteArray AE_CALL slice(size_t length, Usage usage = Usage::SHARED) const;
		inline ByteArray AE_CALL slice(Usage usage = Usage::SHARED) const;

		inline size_t AE_CALL getCapacity() const;
		inline void AE_CALL setCapacity(size_t capacity);

		inline size_t AE_CALL getLength() const;
		inline void AE_CALL setLength(size_t len);

		inline size_t AE_CALL getPosition() const;
		inline void AE_CALL setPosition(size_t pos);

		inline void AE_CALL seekBegin();
		inline void AE_CALL seekEnd();

		inline size_t AE_CALL getBytesAvailable() const;

		uint64_t AE_CALL readDynamicUInt64();
		void AE_CALL writeDynamicUInt64(uint64_t value);

		int64_t AE_CALL readInt(uint8_t numBytes);
		uint64_t AE_CALL readUInt(uint8_t numBytes);
		void AE_CALL writeInt(uint8_t numBytes, int64_t value);
		inline void AE_CALL writeUInt(uint8_t numBytes, uint64_t value);

		size_t AE_CALL readBytes(uint8_t* bytes, size_t length = (std::numeric_limits<size_t>::max)());
		size_t AE_CALL readBytes(ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)());
		inline void AE_CALL writeBytes(const uint8_t* bytes, size_t length);
		size_t AE_CALL writeBytes(const ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)());

		//begin, size, position
		std::tuple<size_t, size_t, size_t> AE_CALL readString(size_t begin, size_t size, bool chechBOM = false) const;
		//begin, size
		inline std::tuple<size_t, size_t> AE_CALL readString(size_t size, bool chechBOM = false);
		inline std::string AE_CALL readString(bool chechBOM = false);
		inline std::string_view AE_CALL readStringView(bool chechBOM = false);
		inline void AE_CALL writeString(const std::string& str);
		inline void AE_CALL writeString(const std::string_view& str);
		void AE_CALL writeString(const char* str, size_t size);
		inline void AE_CALL writeString(const char* str);

		template<typename T>
		T AE_CALL read() {
			if constexpr (
				std::is_same_v<T, bool> || 
				std::is_same_v<T, int8_t> ||
				std::is_same_v<T, uint8_t> ||
				std::is_same_v<T, int16_t> ||
				std::is_same_v<T, uint16_t> ||
				std::is_same_v<T, int32_t> ||
				std::is_same_v<T, uint32_t> ||
				std::is_same_v<T, int64_t> ||
				std::is_same_v<T, uint64_t> ||
				std::is_same_v<T, f32> ||
				std::is_same_v<T, f64>) {
				if constexpr (sizeof(T) == 1) {
					if (_position < _length) {
						return *(T*)&_data[_position++];
					} else {
						_position = _length;
						return (T)0;
					}
				} else {
					const uint32_t len = sizeof(T);
					if (_position + len > _length) {
						_position = _length;
						return (T)0;
					}

					if (_needReverse) {
						T v;
						auto p = (uint8_t*)&v;
						for (int32_t i = len - 1; i >= 0; --i) p[i] = _data[_position++];
						return v;
					} else {
						T v = *(T*)&_data[_position];
						_position += len;
						return v;
					}
				}
			} else {
				static_assert(false, "ByteArray read<T>, unsupported type");
			}
		}

		template<typename T>
		inline void AE_CALL write(const T& value) {
			if constexpr (
				std::is_same_v<T, bool> ||
				std::is_same_v<T, int8_t> ||
				std::is_same_v<T, uint8_t> ||
				std::is_same_v<T, int16_t> ||
				std::is_same_v<T, uint16_t> ||
				std::is_same_v<T, int32_t> ||
				std::is_same_v<T, uint32_t> ||
				std::is_same_v<T, int64_t> ||
				std::is_same_v<T, uint64_t> ||
				std::is_same_v<T, f32> ||
				std::is_same_v<T, f64>) {
				_write((uint8_t*)&value, sizeof(T));
			} else {
				static_assert(false, "ByteArray write<T>, unsupported type");
			}
		}

		inline void AE_CALL writePadding(size_t length);

		inline std::tuple<uint16_t, uint16_t> AE_CALL readTwoUInt12();
		inline void AE_CALL writeTwoUInt12(uint16_t value1, uint16_t value2);

		inline std::tuple<int16_t, int16_t> AE_CALL readTwoInt12();
		inline void AE_CALL writeTwoInt12(int16_t value1, int16_t value2);

		void AE_CALL popFront(size_t len);
		void AE_CALL popBack(size_t len);
		void AE_CALL insert(size_t len);

		inline static bool isEqual(const ByteArray& data1, const ByteArray& data2);
		static bool isEqual(const uint8_t* data1, const uint8_t* data2, size_t len);

	private:
		Usage _usage;
		bool _needReverse;
		uint8_t* _data;
		std::endian _endian;
		size_t _position;
		size_t _length;
		size_t _capacity;

		inline void AE_CALL _read(uint8_t* p, uint32_t len);
		inline void AE_CALL _write(const uint8_t* p, uint32_t len);

		void AE_CALL _resize(size_t len);
		inline void AE_CALL _dilatation(size_t size);
		inline void AE_CALL _checkLength(size_t len);

		inline uint8_t AE_CALL _bomOffset(size_t pos) const;
	};
}

#include "ByteArray.inl"