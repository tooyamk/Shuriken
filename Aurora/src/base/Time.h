#pragma once

#include "base/Aurora.h"
#include <chrono>

namespace aurora {
	class AE_TEMPLATE_DLL Time {
	public:
		Time() = delete;
		Time(const Time&) = delete;
		Time(Time&&) = delete;

		/**
		 * system_clock : utc time.
		 * steady_clock : monotonic time.
		 */
		template<typename To = std::chrono::nanoseconds, typename Clock = std::chrono::steady_clock>
		inline static i64 AE_CALL now() {
			return std::chrono::time_point_cast<To>(Clock::now()).time_since_epoch().count();
		}
	};
}