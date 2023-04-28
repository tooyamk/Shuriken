#pragma once

#include "srk/Core.h"

namespace srk::literals {
	inline constexpr int8_t operator"" _i8(unsigned long long n) noexcept {
		return (int8_t)n;
	}
	inline constexpr uint8_t operator"" _ui8(unsigned long long n) noexcept {
		return (uint8_t)n;
	}
	inline constexpr int16_t operator"" _i16(unsigned long long n) noexcept {
		return (int16_t)n;
	}
	inline constexpr uint16_t operator"" _ui16(unsigned long long n) noexcept {
		return (uint16_t)n;
	}
	inline constexpr int32_t operator"" _i32(unsigned long long n) noexcept {
		return (int32_t)n;
	}
	inline constexpr uint32_t operator"" _ui32(unsigned long long n) noexcept {
		return (uint32_t)n;
	}
	inline constexpr int64_t operator"" _i64(unsigned long long n) noexcept {
		return (int64_t)n;
	}
	inline constexpr uint64_t operator"" _ui64(unsigned long long n) noexcept {
		return (uint64_t)n;
	}
	inline constexpr size_t operator"" _uz(unsigned long long n) noexcept {
		return (size_t)n;
	}
}