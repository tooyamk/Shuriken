#include "Memory.h"

namespace srk {
	void* Memory::find(void* data, size_t dataLength, const void* compare, size_t compareLength, size_t stepLength) {
		if (stepLength < 1) stepLength = 1;

		if (compareLength) {
			auto buf = (uint8_t*)data;

			do {
				if (dataLength < compareLength) return nullptr;
				if (!memcmp(buf, compare, compareLength)) return buf;
				if (dataLength < stepLength) return nullptr;

				buf += stepLength;
				dataLength -= stepLength;
			} while (true);
		} else {
			return data;
		}
	}
}