#pragma once

#include "base/LowLevel.h"

namespace aurora::hash {
	class AE_DLL MD5 {
	public:
		MD5();
		std::string hash(const ui8* input, ui32 length);

	private:
		void init();
		void update(const ui8* buf, ui32 length);
		void update(const i8* buf, ui32 length);
		MD5& finalize();
		std::string hexdigest() const;

		inline static const ui32 BLOCK_SIZE = 64;

		void transform(const ui8 block[BLOCK_SIZE]);
		static void decode(ui32 output[], const ui8 input[], ui32 len);
		static void encode(ui8 output[], const ui32 input[], ui32 len);

		ui8 buffer[BLOCK_SIZE];
		ui32 count[2];
		ui32 state[4];
		ui8 digest[16];

		inline static ui32 F(ui32 x, ui32 y, ui32 z) {
			return (x & y) | ((~x) & z);
		}
		inline static ui32 G(ui32 x, ui32 y, ui32 z) {
			return (x & z) | (y & ~z);
		}
		inline static ui32 H(ui32 x, ui32 y, ui32 z) {
			return x ^ y ^ z;
		}
		inline static ui32 I(ui32 x, ui32 y, ui32 z) {
			return y ^ (x | ~z);
		}
		inline static ui32 rotate_left(ui32 x, i32 n) {
			return (x << n) | (x >> (32 - n));
		}
		inline static void FF(ui32 &a, ui32 b, ui32 c, ui32 d, ui32 x, ui32 s, ui32 ac) {
			a = rotate_left(a + F(b, c, d) + x + ac, s) + b;
		}
		inline static void GG(ui32 &a, ui32 b, ui32 c, ui32 d, ui32 x, ui32 s, ui32 ac) {
			a = rotate_left(a + G(b, c, d) + x + ac, s) + b;
		}
		inline static void HH(ui32 &a, ui32 b, ui32 c, ui32 d, ui32 x, ui32 s, ui32 ac) {
			a = rotate_left(a + H(b, c, d) + x + ac, s) + b;
		}
		inline static void II(ui32 &a, ui32 b, ui32 c, ui32 d, ui32 x, ui32 s, ui32 ac) {
			a = rotate_left(a + I(b, c, d) + x + ac, s) + b;
		}
	};
}
