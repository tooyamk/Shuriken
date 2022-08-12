#include "String.h"

namespace srk {
	bool String::isUTF8(const char* data, size_t len) {
		const auto buf = (uint8_t*)data;
		size_t i = 0;
		while (i < len) {
			if (auto c = buf[i]; c < 0x80) {
				++i;
			} else if (c < 0xC0) {
				return false;
			} else if (c < 0xE0) {
				if (i >= len - 1) break;
				if ((buf[i + 1] & 0xC0) != 0x80) return false;
				i += 2;
			} else if (c < 0xF0) {
				if (i >= len - 2) break;
				if ((buf[i + 1] & 0xC0) != 0x80 || (buf[i + 2] & 0xC0) != 0x80) return false;
				i += 3;
			} else {
				return false;
			}
		}
		return true;
	}

	std::string String::toString(const uint8_t* value, size_t size) {
		std::string str(size << 1, 0);

		for (size_t i = 0; i < size; ++i) {
			auto q = value[i] >> 4;
			auto r = value[i] & 0b1111;
			size_t idx = i << 1;

			str[idx] = (q < 10 ? '0' : '7') + q;

			++idx;
			str[idx] = (r < 10 ? '0' : '7') + r;
		}
		return std::move(str);
	}

	bool String::equal(const char* str1, const char* str2) {
		if (str1 == str2) return true;
		
		size_t i = 0;
		do {
			if (str1[i] != str2[i]) return false;
			if (str1[i] == 0) return true;
			++i;
		} while(true);
	}
}