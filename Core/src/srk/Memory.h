#pragma once

#include "srk/Core.h"

namespace srk {
	class SRK_CORE_DLL Memory {
	public:
		Memory() = delete;

		static void* SRK_CALL find(void* data, size_t dataLength, const void* compare, size_t compareLength, size_t stepLength = 1);

		inline static const void* SRK_CALL find(const void* data, size_t dataLength, const void* compare, size_t compareLength, size_t stepLength = 1) {
			return (const void*)find((void*)data, dataLength, compare, compareLength, stepLength);
		}
	};
}