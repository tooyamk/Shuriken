#pragma once

#include "srk/Global.h"

namespace srk {
	class SRK_CORE_DLL Thread {
	public:
		struct SRK_CORE_DLL JobRange {
			size_t begin;
			size_t count;

			JobRange() :
				begin(0),
				count(0) {}

			JobRange(size_t begin, size_t count) :
				begin(begin),
				count(count) {
			}
		};

		static size_t SRK_CALL calcNeedCount(size_t jobCount, size_t leastJobsPerThread, size_t maxThreadCount);
		static JobRange SRK_CALL calcJobRange(size_t jobCount, size_t threadCount, size_t threadIndex);
	};
}