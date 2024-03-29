#pragma once

#include "srk/Core.h"
#include <string>

namespace srk::hash {
	class SRK_CORE_DLL MD5 {
	public:
		MD5();
		std::string SRK_CALL calc(const void* input, size_t length);

	private:
		void SRK_CALL _init();
		void SRK_CALL _update(const uint8_t* buf, size_t length);
		void SRK_CALL _finalize();
		std::string SRK_CALL _hexdigest() const;

		inline static const uint32_t BLOCK_SIZE = 64;

		void SRK_CALL _transform(const uint8_t block[BLOCK_SIZE]);
		static void SRK_CALL _decode(uint32_t output[], const uint8_t* input, size_t len);
		static void SRK_CALL _encode(uint8_t output[], const uint32_t input[], size_t len);

		uint8_t _buffer[BLOCK_SIZE];
		uint32_t _count[2];
		uint32_t _state[4];
		uint8_t _digest[16];

		inline static uint32_t SRK_CALL _F(uint32_t x, uint32_t y, uint32_t z) {
			return (x & y) | ((~x) & z);
		}
		inline static uint32_t SRK_CALL _G(uint32_t x, uint32_t y, uint32_t z) {
			return (x & z) | (y & ~z);
		}
		inline static uint32_t SRK_CALL _H(uint32_t x, uint32_t y, uint32_t z) {
			return x ^ y ^ z;
		}
		inline static uint32_t SRK_CALL _I(uint32_t x, uint32_t y, uint32_t z) {
			return y ^ (x | ~z);
		}
		inline static uint32_t SRK_CALL _rotate_left(uint32_t x, int32_t n) {
			return (x << n) | (x >> (32 - n));
		}
		inline static void SRK_CALL _FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = _rotate_left(a + _F(b, c, d) + x + ac, s) + b;
		}
		inline static void SRK_CALL _GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = _rotate_left(a + _G(b, c, d) + x + ac, s) + b;
		}
		inline static void SRK_CALL _HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = _rotate_left(a + _H(b, c, d) + x + ac, s) + b;
		}
		inline static void SRK_CALL _II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = _rotate_left(a + _I(b, c, d) + x + ac, s) + b;
		}
	};
}
