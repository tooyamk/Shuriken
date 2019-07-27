#include "CRC.h"

namespace aurora::hash {
	uint16_t CRC::CRC16(const uint8_t* data, uint16_t len) {
		uint16_t crc = 0xFFFF;
		for (uint16_t i = 0; i < len; ++i) crc = ((crc << 8) & 0xFFFF) ^ _table16[(crc >> 8) ^ (data[i] & 0xFF)];
		return crc;
	}

	uint32_t CRC::CRC32(const uint8_t* data, uint32_t len) {
		uint32_t uiCRC32 = (std::numeric_limits<uint32_t>::max)();
		for (uint32_t i = 0; i < len; ++i) uiCRC32 = ((uiCRC32 >> 8) & 0x00FFFFFFui32) ^ _table32[(uiCRC32 ^ (uint32_t)data[i]) & 0xFF];
		return (uiCRC32 ^ (std::numeric_limits<uint32_t>::max)());
	}

	uint64_t CRC::CRC64(const uint8_t* data, uint32_t len) {
		auto crc = (std::numeric_limits<uint64_t>::max)();
		for (uint32_t i = 0; i < len; ++i) crc = _table64[((uint32_t)(crc >> 56) ^ (uint32_t)data[i]) & 0xFF] ^ (crc << 8);
		return crc ^= (std::numeric_limits<uint64_t>::max)();
	}

	void CRC::CRC64StreamIteration(uint64_t& crc, const uint8_t* data, uint32_t len) {
		for (uint32_t i = 0; i < len; ++i) crc = _table64[((uint32_t)(crc >> 56) ^ (uint32_t)data[i]) & 0xFF] ^ (crc << 8);
	}
}