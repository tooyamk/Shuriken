#pragma once

#include "base/LowLevel.h"
#include <algorithm>

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


		enum class Endian : ui8 {
			BIG,
			LITTLE
		};


		ByteArray(const ByteArray&) = delete;
		ByteArray& operator=(const ByteArray&) = delete;
		ByteArray(ByteArray&& bytes);
		ByteArray(ui32 capacity = 0, ui32 length = 0);
		ByteArray(i8* bytes, ui32 size, ExtMemMode extMode = ExtMemMode::EXT);
		ByteArray(i8* bytes, ui32 length, ui32 capacity, ExtMemMode extMode = ExtMemMode::EXT);
		ByteArray& operator=(ByteArray&& value);
		~ByteArray();

		inline operator bool() const;

		void AE_CALL dispose(bool free = true);

		inline void AE_CALL clear();
		inline void AE_CALL detach();

		inline bool AE_CALL isValid() const;

		inline Endian AE_CALL getEndian() const;
		inline void AE_CALL setEndian(Endian endian);

		inline const i8* AE_CALL getBytes() const;

		inline ByteArray AE_CALL slice(ui32 start, ui32 length) const;

		inline ui32 AE_CALL getCapacity() const;
		inline void AE_CALL setCapacity(ui32 capacity);

		inline ui32 AE_CALL getLength() const;
		inline void AE_CALL setLength(ui32 len);

		inline ui32 AE_CALL getPosition() const;
		inline void AE_CALL setPosition(ui32 pos);

		inline void AE_CALL seekBegin();
		inline void AE_CALL seekEnd();

		inline ui32 AE_CALL getBytesAvailable() const;

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

		ui32 AE_CALL readBytes(i8* bytes, ui32 offset = 0, ui32 length = UINT_MAX);
		ui32 AE_CALL readBytes(ByteArray& ba, ui32 offset = 0, ui32 length = UINT_MAX);
		inline void AE_CALL writeBytes(const i8* bytes, ui32 offset, ui32 length);
		ui32 AE_CALL writeBytes(const ByteArray& ba, ui32 offset = 0, ui32 length = UINT_MAX);

		ui32 AE_CALL readStringLength(ui32 begin, ui32 size, bool chechBOM = false) const;
		inline ui32 AE_CALL readStringLength(ui32 size, bool chechBOM = false) const;
		inline std::string AE_CALL readString(bool chechBOM = false);
		inline std::string_view AE_CALL readStringView(bool chechBOM = false);
		inline void AE_CALL writeString(const std::string& str);
		void AE_CALL writeString(const i8* str, ui32 size);

		inline bool AE_CALL readBool();
		inline void AE_CALL writeBool(bool b);

		inline void AE_CALL readTwoUInt12(ui16& value1, ui16& value2);
		inline void AE_CALL writeTwoUInt12(ui16 value1, ui16 value2);

		inline void AE_CALL readTwoInt12(i16& value1, i16& value2);
		inline void AE_CALL writeTwoInt12(i16 value1, i16 value2);

		void AE_CALL popFront(ui32 len);
		void AE_CALL popBack(ui32 len);
		void AE_CALL insert(ui32 len);

		inline static bool isEqual(const ByteArray& data1, const ByteArray& data2);
		static bool isEqual(const i8* data1, ui32 data1Len, const i8* data2, ui32 data2Len);

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

		static const ui16 TEST_ENDIAN_VALUE = 0x00FF;

		Endian _endian;
		Mode _mode;
		bool _needReverse;
		i8* _data;
		ui32 _position;
		ui32 _length;
		ui32 _capacity;

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

		void AE_CALL _resize(ui32 len);
		inline void AE_CALL _dilatation(ui32 size);
		inline void AE_CALL _checkLength(ui32 len);

		inline ui8 AE_CALL _bomOffset(ui32 pos) const;

	public:
		static const Endian SYS_ENDIAN;
	};
}

#include "ByteArray.inl"