#pragma once

#include "base/LowLevel.h"

namespace aurora::hash {
	class AE_DLL MD5 {
	public:
		MD5();
		std::string hash(const uint8_t* input, uint32_t length);

	private:
		void init();
		void update(const uint8_t* buf, uint32_t length);
		MD5& finalize();
		std::string hexdigest() const;

		inline static const uint32_t BLOCK_SIZE = 64;

		void transform(const uint8_t block[BLOCK_SIZE]);
		static void decode (uint32_t output[], const uint8_t input[], uint32_t len);
		static void encode(uint8_t output[], const uint32_t input[], uint32_t len);

		uint8_t buffer[BLOCK_SIZE];
		uint32_t count[2];
		uint32_t state[4];
		uint8_t digest[16];

		inline static uint32_t F (uint32_t x, uint32_t y, uint32_t z) {
			return (x & y) | ((~x) & z);
		}
		inline static uint32_t G (uint32_t x, uint32_t y, uint32_t z) {
			return (x & z) | (y & ~z);
		}
		inline static uint32_t H (uint32_t x, uint32_t y, uint32_t z) {
			return x ^ y ^ z;
		}
		inline static uint32_t I (uint32_t x, uint32_t y, uint32_t z) {
			return y ^ (x | ~z);
		}
		inline static uint32_t rotate_left (uint32_t x, int32_t n) {
			return (x << n) | (x >> (32 - n));
		}
		inline static void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = rotate_left(a + F(b, c, d) + x + ac, s) + b;
		}
		inline static void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = rotate_left(a + G(b, c, d) + x + ac, s) + b;
		}
		inline static void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = rotate_left(a + H(b, c, d) + x + ac, s) + b;
		}
		inline static void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
			a = rotate_left(a + I(b, c, d) + x + ac, s) + b;
		}
	};
}
