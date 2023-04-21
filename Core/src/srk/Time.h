#pragma once

#include "srk/Core.h"
#include <chrono>

namespace srk {
	class Time {
	public:
		/**
		 * system_clock : utc time.
		 * steady_clock : monotonic time.
		 */
		template<typename Duration = std::chrono::milliseconds, typename Clock = std::chrono::steady_clock>
		inline static int64_t SRK_CALL now() {
			return std::chrono::time_point_cast<Duration>(Clock::now()).time_since_epoch().count();
		}
	};
}