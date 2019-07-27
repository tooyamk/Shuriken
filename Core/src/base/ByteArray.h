#pragma once

#include "base/LowLevel.h"
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


		//enum class CompressionAlgorithm : uint8_t {
		//	ZLIB
		//};


		ByteArray(const ByteArray&) = delete;
		ByteArray& AE_CALL operator=(const ByteArray&) = delete;
		ByteArray(ByteArray&& bytes);
		ByteArray(size_t capacity = 0, size_t length = 0);
		ByteArray(uint8_t* bytes, size_t size, Usage extMode = Usage::SHARED);
		ByteArray(uint8_t* bytes, size_t capacity, size_t length, Usage extMode = Usage::SHARED);
		ByteArray& AE_CALL operator=(ByteArray&& value);
		~ByteArray();

		inline AE_CALL operator bool() const;

		void AE_CALL dispose(bool free = true);

		inline void AE_CALL clear();
		inline void AE_CALL detach();

		inline bool AE_CALL isValid() const;

		inline Endian AE_CALL getEndian() const;
		inline void AE_CALL setEndian(Endian endian);

		inline uint8_t* AE_CALL getBytes();
		inline const uint8_t* AE_CALL getBytes() const;

		inline ByteArray AE_CALL slice(size_t start, size_t length) const;

		inline size_t AE_CALL getCapacity() const;
		inline void AE_CALL setCapacity(size_t capacity);

		inline size_t AE_CALL getLength() const;
		inline void AE_CALL setLength(size_t len);

		inline size_t AE_CALL getPosition() const;
		inline void AE_CALL setPosition(size_t pos);

		inline void AE_CALL seekBegin();
		inline void AE_CALL seekEnd();

		inline size_t AE_CALL getBytesAvailable() const;

		inline int8_t AE_CALL readInt8();
		inline uint8_t AE_CALL readUInt8();
		inline void AE_CALL writeInt8(int8_t value);
		inline void AE_CALL writeUInt8(uint8_t value);

		inline uint16_t AE_CALL readInt16();
		inline uint16_t AE_CALL readUInt16();
		inline void AE_CALL writeInt16(uint16_t value);
		inline void AE_CALL writeUInt16(uint16_t value);

		inline int32_t AE_CALL readInt32();
		inline uint32_t AE_CALL readUInt32();
		inline void AE_CALL writeInt32(int32_t value);
		inline void AE_CALL writeUInt32(uint32_t value);

		inline f32 AE_CALL readFloat32();
		inline void AE_CALL writeFloat32(f32 value);

		inline f64 AE_CALL readFloat64();
		inline void AE_CALL writeFloat64(const f64& value);

		inline int64_t AE_CALL readInt64();
		inline void AE_CALL writeInt64(const int64_t& value);

		inline uint64_t AE_CALL readUInt64();
		inline void AE_CALL writeUInt64(const uint64_t& value);

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
		void AE_CALL writeString(const char* str, size_t size);
		inline void AE_CALL writeStringView(const std::string_view& str);

		inline bool AE_CALL readBool();
		inline void AE_CALL writeBool(bool b);

		inline void AE_CALL readTwoUInt12(uint16_t& value1, uint16_t& value2);
		inline void AE_CALL writeTwoUInt12(uint16_t value1, uint16_t value2);

		inline void AE_CALL readTwoInt12(int16_t& value1, int16_t& value2);
		inline void AE_CALL writeTwoInt12(int16_t value1, int16_t value2);

		void AE_CALL popFront(size_t len);
		void AE_CALL popBack(size_t len);
		void AE_CALL insert(size_t len);

		inline static bool isEqual(const ByteArray& data1, const ByteArray& data2);
		static bool isEqual(const uint8_t* data1, size_t data1Len, const uint8_t* data2, size_t data2Len);

	private:
		static const uint16_t INT12 = 1 << 12;
		static const uint16_t INT12_MAX = (INT12 >> 1) - 1;
		static const int32_t INT24 = 1 << 24;
		static const int32_t INT24_MAX = (INT24 >> 1) - 1;
		static const uint64_t INT40 = 0x10000000000ui64;//1 << 40
		static const uint64_t INT40_MAX = (INT40 / 2) - 1;
		static const uint64_t INT48 = 0x1000000000000ui64;//1 << 48
		static const uint64_t INT48_MAX = (INT48 / 2) - 1;
		static const uint64_t INT56 = 0x100000000000000ui64;//1 << 56
		static const uint64_t INT56_MAX = (INT56 / 2) - 1;

		Endian _endian;
		Usage _usage;
		bool _needReverse;
		uint8_t* _data;
		size_t _position;
		size_t _length;
		size_t _capacity;

		template<typename K>
		K AE_CALL _read() {
			const uint32_t len = sizeof(K);
			if (_position + len > _length) {
				_position = _length;
				return (K)0;
			}

			if (_needReverse) {
				K v;
				int8_t* p = (int8_t*)&v;
				for (int8_t i = len - 1; i >= 0; --i) p[i] = _data[_position++];
				return v;
			} else {
				K* p = (K*)&_data[_position];
				_position += len;
				return *p;
			}
		}
		inline void AE_CALL _read(uint8_t* p, uint32_t len);
		inline void AE_CALL _write(const uint8_t* p, uint32_t len);

		void AE_CALL _resize(size_t len);
		inline void AE_CALL _dilatation(size_t size);
		inline void AE_CALL _checkLength(size_t len);

		inline uint8_t AE_CALL _bomOffset(size_t pos) const;
	};
}

#include "ByteArray.inl"