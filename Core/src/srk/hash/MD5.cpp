#include "MD5.h"

namespace srk::hash {
	MD5::MD5() {
	}

	void MD5::_init() {
		_count[0] = 0;
		_count[1] = 0;

		_state[0] = 0x67452301;
		_state[1] = 0xEFCDAB89;
		_state[2] = 0x98BADCFE;
		_state[3] = 0x10325476;
	}

	void MD5::_decode(uint32_t output[], const uint8_t* input, size_t len) {
		for (size_t i = 0, j = 0; j < len; ++i, j += 4) output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) | (((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
	}

	void MD5::_encode(uint8_t output[], const uint32_t input[], size_t len) {
		for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
			output[j] = input[i] & 0xFF;
			output[j + 1] = (input[i] >> 8) & 0xFF;
			output[j + 2] = (input[i] >> 16) & 0xFF;
			output[j + 3] = (input[i] >> 24) & 0xFF;
		}
	}

	void MD5::_transform(const uint8_t block[BLOCK_SIZE]) {
		const uint32_t S11 = 7;
		const uint32_t S12 = 12;
		const uint32_t S13 = 17;
		const uint32_t S14 = 22;
		const uint32_t S21 = 5;
		const uint32_t S22 = 9;
		const uint32_t S23 = 14;
		const uint32_t S24 = 20;
		const uint32_t S31 = 4;
		const uint32_t S32 = 11;
		const uint32_t S33 = 16;
		const uint32_t S34 = 23;
		const uint32_t S41 = 6;
		const uint32_t S42 = 10;
		const uint32_t S43 = 15;
		const uint32_t S44 = 21;

		uint32_t a = _state[0], b = _state[1], c = _state[2], d = _state[3], x[16];
		_decode(x, block, BLOCK_SIZE);

		/* Round 1 */
		_FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
		_FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
		_FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
		_FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
		_FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
		_FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
		_FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
		_FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
		_FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
		_FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
		_FF(c, d, a, b, x[10], S13, 0xFFff5bb1); /* 11 */
		_FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
		_FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
		_FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
		_FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
		_FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

		/* Round 2 */
		_GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
		_GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
		_GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
		_GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
		_GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
		_GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
		_GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
		_GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
		_GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
		_GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
		_GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
		_GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
		_GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
		_GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
		_GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
		_GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

		/* Round 3 */
		_HH(a, b, c, d, x[5], S31, 0xFFfa3942); /* 33 */
		_HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
		_HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
		_HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
		_HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
		_HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
		_HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
		_HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
		_HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
		_HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
		_HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
		_HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
		_HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
		_HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
		_HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
		_HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

		/* Round 4 */
		_II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
		_II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
		_II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
		_II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
		_II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
		_II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
		_II(c, d, a, b, x[10], S43, 0xFFeff47d); /* 55 */
		_II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
		_II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
		_II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
		_II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
		_II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
		_II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
		_II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
		_II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
		_II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

		_state[0] += a;
		_state[1] += b;
		_state[2] += c;
		_state[3] += d;

		memset(x, 0, sizeof(x));
	}

	void MD5::_update(const uint8_t* input, size_t length) {
		uint32_t index = _count[0] / 8 % BLOCK_SIZE;

		if ((_count[0] += (length << 3)) < (length << 3)) ++_count[1];
		_count[1] += (length >> 29);

		uint32_t firstpart = 64 - index;

		uint32_t i;

		if (length >= firstpart) {
			memcpy(&_buffer[index], input, firstpart);
			_transform(_buffer);

			for (i = firstpart; i + BLOCK_SIZE <= length; i += BLOCK_SIZE) _transform(&input[i]);

			index = 0;
		} else {
			i = 0;
		}

		memcpy(&_buffer[index], &input[i], length - i);
	}

	void MD5::_finalize() {
		static uint8_t padding[64] = {
			0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		uint8_t bits[8];
		_encode(bits, _count, 8);

		uint32_t index = _count[0] / 8 % 64;
		uint32_t padLen = (index < 56) ? (56 - index) : (120 - index);
		_update(padding, padLen);

		_update(bits, 8);

		_encode(_digest, _state, 16);

		memset(_buffer, 0, sizeof(_buffer));
		memset(_count, 0, sizeof(_count));
	}

	std::string MD5::_hexdigest() const {
		char buf[33];
		for (uint8_t i = 0; i < 16; ++i) sprintf(buf + i * 2, "%02x", _digest[i]);
		buf[32] = 0;

		return std::string(buf);
	}

	std::string MD5::calc(const void* input, size_t length) {
		_init();
		_update((const uint8_t*)input, length);
		_finalize();
		return _hexdigest();
	}
}