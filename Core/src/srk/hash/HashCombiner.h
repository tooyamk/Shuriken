#pragma once

#include "srk/Core.h"

namespace srk::hash {
	class SRK_CORE_DLL HashCombiner {
	public:
		template<typename T, typename... Others>
		static std::size_t& SRK_CALL combine(std::size_t& seed, const T& v, const Others&... others) {
			seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
			(combine(seed, others), ...);
			return seed;
		}
	};
}