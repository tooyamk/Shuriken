#include "CRC.h"

namespace aurora::hash {
	ui16 CRC::CRC16(const i8* data, ui16 len) {
		ui16 crc = 0xFFFF;
		for (ui16 i = 0; i < len; ++i) crc = ((crc << 8) & 0xFFFF) ^ _table16[(crc >> 8) ^ (data[i] & 0xFF)];
		return crc;
	}

	ui32 CRC::CRC32(const i8* data, ui32 len) {
		ui32 uiCRC32 = 0xFFFFFFFF;
		for (ui32 i = 0; i < len; ++i) uiCRC32 = ((uiCRC32 >> 8) & 0x00FFFFFFui32) ^ _table32[(uiCRC32 ^ (ui32)data[i]) & 0xFF];
		return (uiCRC32 ^ 0xFFFFFFFFui32);
	}

	ui64 CRC::CRC64(const i8* data, ui32 len) {
		auto crc = 0xFFFFFFFFFFFFFFFFui64;
		for (ui32 i = 0; i < len; ++i) crc = _table64[((ui32)(crc >> 56) ^ (ui32)data[i]) & 0xFF] ^ (crc << 8);
		return crc ^= 0xFFFFFFFFFFFFFFFFui64;
	}

	void CRC::CRC64StreamIteration(ui64& crc, const i8* data, ui32 len) {
		for (ui32 i = 0; i < len; ++i) crc = _table64[((ui32)(crc >> 56) ^ (ui32)data[i]) & 0xFF] ^ (crc << 8);
	}
}