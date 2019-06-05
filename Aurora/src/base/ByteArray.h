#pragma once

#include "base/LowLevel.h"
#include <algorithm>
#include <string_view>

namespace aurora {
	class AE_DLL ByteArray {
	public:
		enum class ExtMemMode : ui8 {
			EXT,
			COPY,
			EXCLUSIVE
		};


		//enum class CompressionAlgorithm : ui8 {
		//	ZLIB
		//};


		ByteArray(const ByteArray&) = delete;
		ByteArray& operator=(const ByteArray&) = delete;
		ByteArray(ByteArray&& bytes);
		ByteArray(size_t capacity = 0, size_t length = 0);
		ByteArray(i8* bytes, size_t size, ExtMemMode extMode = ExtMemMode::EXT);
		ByteArray(i8* bytes, size_t length, size_t capacity, ExtMemMode extMode = ExtMemMode::EXT);
		ByteArray& AE_CALL operator=(ByteArray&& value);
		~ByteArray();

		inline AE_CALL operator bool() const;

		void AE_CALL dispose(bool free = true);

		inline void AE_CALL clear();
		inline void AE_CALL detach();

		inline bool AE_CALL isValid() const;

		inline Endian AE_CALL getEndian() const;
		inline void AE_CALL setEndian(Endian endian);

		inline const i8* AE_CALL getBytes() const;

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

		inline i8 AE_CALL readInt8();
		inline ui8 AE_CALL readUInt8();
		inline void AE_CALL writeInt8(i8 value);
		inline void AE_CALL writeUInt8(ui8 value);

		inline i16 AE_CALL readInt16();
		inline ui16 AE_CALL readUInt16();
		inline void AE_CALL writeInt16(i16 value);
		inline void AE_CALL writeUInt16(ui16 value);

		inline i32 AE_CALL readInt32();
		inline ui32 AE_CALL readUInt32();
		inline void AE_CALL writeInt32(i32 value);
		inline void AE_CALL writeUInt32(ui32 value);

		inline f32 AE_CALL readFloat32();
		inline void AE_CALL writeFloat32(f32 value);

		inline f64 AE_CALL readFloat64();
		inline void AE_CALL writeFloat64(const f64& value);

		inline i64 AE_CALL readInt64();
		inline void AE_CALL writeInt64(const i64& value);

		inline ui64 AE_CALL readUInt64();
		inline void AE_CALL writeUInt64(const ui64& value);

		i64 AE_CALL readInt(ui8 numBytes);
		ui64 AE_CALL readUInt(ui8 numBytes);
		void AE_CALL writeInt(ui8 numBytes, i64 value);
		inline void AE_CALL writeUInt(ui8 numBytes, ui64 value);

		size_t AE_CALL readBytes(i8* bytes, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)());
		size_t AE_CALL readBytes(ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)());
		inline void AE_CALL writeBytes(const i8* bytes, size_t offset, size_t length);
		size_t AE_CALL writeBytes(const ByteArray& ba, size_t offset = 0, size_t length = (std::numeric_limits<size_t>::max)());

		size_t AE_CALL readStringLength(size_t begin, size_t size, bool chechBOM = false) const;
		inline size_t AE_CALL readStringLength(size_t size, bool chechBOM = false) const;
		inline std::string AE_CALL readString(bool chechBOM = false);
		inline std::string_view AE_CALL readStringView(bool chechBOM = false);
		inline void AE_CALL writeString(const std::string& str);
		void AE_CALL writeString(const i8* str, size_t size);

		inline bool AE_CALL readBool();
		inline void AE_CALL writeBool(bool b);

		inline void AE_CALL readTwoUInt12(ui16& value1, ui16& value2);
		inline void AE_CALL writeTwoUInt12(ui16 value1, ui16 value2);

		inline void AE_CALL readTwoInt12(i16& value1, i16& value2);
		inline void AE_CALL writeTwoInt12(i16 value1, i16 value2);

		void AE_CALL popFront(size_t len);
		void AE_CALL popBack(size_t len);
		void AE_CALL insert(size_t len);

		inline static bool isEqual(const ByteArray& data1, const ByteArray& data2);
		static bool isEqual(const i8* data1, size_t data1Len, const i8* data2, size_t data2Len);

	private:
		enum class Mode : ui8 {
			MEM,
			EXT
		};


		static const i16 INT12 = 1 << 12;
		static const i16 INT12_MAX = (INT12 >> 1) - 1;
		static const i32 INT24 = 1 << 24;
		static const i32 INT24_MAX = (INT24 >> 1) - 1;
		static const ui64 INT40 = 0x10000000000ui64;//1 << 40
		static const ui64 INT40_MAX = (INT40 / 2) - 1;
		static const ui64 INT48 = 0x1000000000000ui64;//1 << 48
		static const ui64 INT48_MAX = (INT48 / 2) - 1;
		static const ui64 INT56 = 0x100000000000000ui64;//1 << 56
		static const ui64 INT56_MAX = (INT56 / 2) - 1;

		Endian _endian;
		Mode _mode;
		bool _needReverse;
		i8* _data;
		size_t _position;
		size_t _length;
		size_t _capacity;

		template<typename K>
		K AE_CALL _read() {
			const ui32 len = sizeof(K);
			if (_position + len > _length) return (K)0;

			if (_needReverse) {
				K v;
				i8* p = (i8*)&v;
				for (i8 i = len - 1; i >= 0; --i) p[i] = _data[_position++];
				return v;
			} else {
				K* p = (K*)&_data[_position];
				_position += len;
				return *p;
			}
		}
		inline void AE_CALL _read(i8* p, ui32 len);
		inline void AE_CALL _write(const i8* p, ui32 len);

		void AE_CALL _resize(size_t len);
		inline void AE_CALL _dilatation(size_t size);
		inline void AE_CALL _checkLength(size_t len);

		inline ui8 AE_CALL _bomOffset(size_t pos) const;
	};
}

#include "ByteArray.inl"