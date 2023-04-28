#pragma once

#include "srk/Core.h"

namespace srk {
	class SRK_CORE_DLL Bit {
	public:
		Bit() = delete;

		template<size_t Bits>
		requires (Bits <= 64)
		inline static constexpr uint_t<Bits> SRK_CALL uintMax() {
			uint_t<Bits> val = 0;
			for (size_t i = 0; i < Bits; ++i) val |= (uint_t<Bits>)1 << i;
			return val;
		}

		template<size_t Bits>
		inline static constexpr int_t<Bits> SRK_CALL intMax() {
			return uintMax<Bits>() >> 1;
		}

		template<size_t Bits>
		inline static constexpr int_t<Bits> SRK_CALL intMin() {
			return -intMax<Bits>() - 1;
		}

		template<size_t Bytes, bool AlignedAccess>
		requires (Bytes <= 8)
		inline static uint_t<Bytes * 8> SRK_CALL byteswap(const void* val) {
			using T = uint_t<Bytes * 8>;
			auto data = (const uint8_t*)val;

			if constexpr (Bytes == 0) {
				return 0;
			} else if constexpr (Bytes == 1) {
				return data[0];
			} else if constexpr (Bytes == 2) {
#if SRK_COMPILER == SRK_COMPILER_MSVC
				if constexpr (AlignedAccess) {
					uint16_t v;
					memcpy(&v, val, sizeof(v));
					return _byteswap_ushort(v);
				} else {
					return _byteswap_ushort(*((T*)val));
				}
#elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
				if constexpr (AlignedAccess) {
					uint16_t v;
					memcpy(&v, val, sizeof(v));
					return __builtin_bswap16(v);
				} else {
					return __builtin_bswap16(*((T*)val));
				}
#else
				return (T)data[0] << 8 | (T)data[1];
#endif
			} else if constexpr (Bytes == 3) {
				return (T)data[0] << 16 | (T)data[1] << 8 | (T)data[2];
			} else if constexpr (Bytes == 4) {
#if SRK_COMPILER == SRK_COMPILER_MSVC
				if constexpr (AlignedAccess) {
					uint32_t v;
					memcpy(&v, val, sizeof(v));
					return _byteswap_ulong(v);
				} else {
					return _byteswap_ulong(*((T*)val));
				}
#elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
				if constexpr (AlignedAccess) {
					uint32_t v;
					memcpy(&v, val, sizeof(v));
					return __builtin_bswap32(v);
				} else {
					return __builtin_bswap32(*((T*)val));
				}
#else
				return (T)data[0] << 24 | (T)data[1] << 16 | (T)data[2] << 8 | (T)data[3];
#endif
			} else if constexpr (Bytes == 5) {
				return (T)data[0] << 32 | (T)data[1] << 24 | (T)data[2] << 16 | (T)data[3] << 8 | (T)data[4];
			} else if constexpr (Bytes == 6) {
				return (T)data[0] << 40 | (T)data[1] << 32 | (T)data[2] << 24 | (T)data[3] << 16 | (T)data[4] << 8 | (T)data[5];
			} else if constexpr (Bytes == 7) {
				return (T)data[0] << 48 | (T)data[1] << 40 | (T)data[2] << 32 | (T)data[3] << 24 | (T)data[4] << 16 | (T)data[5] << 8 | (T)data[6];
			} else if constexpr (Bytes == 8) {
#if SRK_COMPILER == SRK_COMPILER_MSVC
				if constexpr (AlignedAccess) {
					uint64_t v;
					memcpy(&v, val, sizeof(v));
					return _byteswap_uint64(v);
				} else {
					return _byteswap_uint64(*((T*)val));
				}
#elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
				if constexpr (AlignedAccess) {
					uint64_t v;
					memcpy(&v, val, sizeof(v));
					return __builtin_bswap64(v);
				} else {
					return __builtin_bswap64(*((T*)val));
				}
#else
				return (T)data[0] << 56 | (T)data[1] << 48 | (T)data[2] << 40 | (T)data[3] << 32 | (T)data[4] << 24 | (T)data[5] << 16 | (T)data[6] << 8 | (T)data[7];
#endif
			} else {
				static_assert(Bytes <= 8, "Unexpected integer size");
			}
		}

		template<size_t Bytes>
		requires (Bytes <= 8)
		inline static uint_t<Bytes * 8> SRK_CALL byteswap(uint_t<Bytes * 8> val) {
			return byteswap<Bytes>(&val);
		}

		template<std::floating_point F>
		inline static F SRK_CALL byteswap(F val) {
			auto v = byteswap<sizeof(F)>(&val);
			return *(F*)&v;
		}
	};
}