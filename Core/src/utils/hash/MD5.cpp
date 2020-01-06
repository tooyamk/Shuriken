#include "md5.h"

#include <stdio.h>

namespace aurora::hash {
	MD5::MD5() {
	}

	void MD5::init() {
		_count[0] = 0;
		_count[1] = 0;

		// load magic initialization constants.
		_state[0] = 0x67452301;
		_state[1] = 0xEFCDAB89;
		_state[2] = 0x98BADCFE;
		_state[3] = 0x10325476;
	}

	// decodes input (uint8_t) into output (uint32_t). Assumes len is a multiple of 4.
	void MD5::decode (uint32_t output[], const uint8_t input[], size_t len) {
		for (size_t i = 0, j = 0; j < len; ++i, j += 4)
			output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) |
			(((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
	}

	// encodes input (uint32_t) into output (uint8_t). Assumes len is
	// a multiple of 4.
	void MD5::encode(uint8_t output[], const uint32_t input[], size_t len) {
		for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
			output[j] = input[i] & 0xFF;
			output[j + 1] = (input[i] >> 8) & 0xFF;
			output[j + 2] = (input[i] >> 16) & 0xFF;
			output[j + 3] = (input[i] >> 24) & 0xFF;
		}
	}

	// apply MD5 algo on a block
	void MD5::transform(const uint8_t block[BLOCK_SIZE]) {
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
		decode(x, block, BLOCK_SIZE);

		/* Round 1 */
		FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
		FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
		FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
		FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
		FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
		FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
		FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
		FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
		FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
		FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
		FF(c, d, a, b, x[10], S13, 0xFFff5bb1); /* 11 */
		FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
		FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
		FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
		FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
		FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

		/* Round 2 */
		GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
		GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
		GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
		GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
		GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
		GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
		GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
		GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
		GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
		GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
		GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
		GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
		GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
		GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
		GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
		GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

		/* Round 3 */
		HH(a, b, c, d, x[5], S31, 0xFFfa3942); /* 33 */
		HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
		HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
		HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
		HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
		HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
		HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
		HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
		HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
		HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
		HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
		HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
		HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
		HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
		HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
		HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

		/* Round 4 */
		II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
		II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
		II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
		II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
		II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
		II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
		II(c, d, a, b, x[10], S43, 0xFFeff47d); /* 55 */
		II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
		II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
		II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
		II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
		II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
		II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
		II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
		II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
		II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

		_state[0] += a;
		_state[1] += b;
		_state[2] += c;
		_state[3] += d;

		// Zeroize sensitive information.
		memset(x, 0, sizeof x);
	}

	// MD5 block update operation. Continues an MD5 message-digest
	// operation, processing another message block
	void MD5::update(const uint8_t* input, size_t length) {
		// compute number of bytes mod 64
		uint32_t index = _count[0] / 8 % BLOCK_SIZE;

		// Update number of bits
		if ((_count[0] += (length << 3)) < (length << 3)) ++_count[1];
		_count[1] += (length >> 29);

		// number of bytes we need to fill in buffer
		uint32_t firstpart = 64 - index;

		uint32_t i;

		// transform as many times as possible.
		if (length >= firstpart) {
			// fill buffer first, transform
			memcpy(&_buffer[index], input, firstpart);
			transform(_buffer);

			// transform chunks of blocksize (64 bytes)
			for (i = firstpart; i + BLOCK_SIZE <= length; i += BLOCK_SIZE) transform(&input[i]);

			index = 0;
		} else {
			i = 0;
		}

		// buffer remaining input
		memcpy(&_buffer[index], &input[i], length - i);
	}

	// MD5 finalization. Ends an MD5 message-digest operation, writing the
	// the message digest and zeroizing the context.
	MD5& MD5::finalize() {
		static uint8_t padding[64] = {
			0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		// Save number of bits
		uint8_t bits[8];
		encode(bits, _count, 8);

		// pad out to 56 mod 64.
		uint32_t index = _count[0] / 8 % 64;
		uint32_t padLen = (index < 56) ? (56 - index) : (120 - index);
		update(padding, padLen);

		// Append length (before padding)
		update(bits, 8);

		// Store state in digest
		encode(_digest, _state, 16);

		// Zeroize sensitive information.
		memset(_buffer, 0, sizeof _buffer);
		memset(_count, 0, sizeof _count);

		return *this;
	}

	// return hex representation of digest as string
	std::string MD5::hexdigest() const {
		char buf[33];
		for (uint8_t i = 0; i < 16; ++i) sprintf(buf + i * 2, "%02x", _digest[i]);
		buf[32] = 0;

		return std::string(buf);
	}

	std::string MD5::hash(const uint8_t* input, size_t length) {
		init();
		update(input, length);
		finalize();
		return hexdigest();
	}
}