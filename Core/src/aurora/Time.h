#pragma once

#include "aurora/Global.h"
#include <chrono>

namespace aurora {
	class Time {
	public:
		/**
		 * system_clock : utc time.
		 * steady_clock : monotonic time.
		 */
		template<typename Duration = std::chrono::milliseconds, typename Clock = std::chrono::steady_clock>
		inline static int64_t AE_CALL now() {
			return std::chrono::time_point_cast<Duration>(Clock::now()).time_since_epoch().count();
		}
	};
}