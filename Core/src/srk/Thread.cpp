#include "Thread.h"

namespace srk {
	size_t Thread::calcNeedCount(size_t jobCount, size_t leastJobsPerThread, size_t maxThreadCount) {
		if (jobCount == 0 || leastJobsPerThread == 0 || maxThreadCount == 0) return 0;

		auto count = (jobCount + leastJobsPerThread - 1) / leastJobsPerThread;
		return maxThreadCount >= count ? count : maxThreadCount;
	}

	Thread::JobRange Thread::calcJobRange(size_t jobCount, size_t threadCount, size_t threadIndex) {
		if (jobCount == 0 || threadCount == 0 || threadIndex >= threadCount) return JobRange();

		auto div = std::div((std::make_signed_t<size_t>)jobCount, (std::make_signed_t<size_t>)threadCount);
		if (div.rem == 0) {
			return JobRange(threadIndex * div.quot, div.quot);
		} else {
			return threadIndex < div.rem ? JobRange(threadIndex * div.quot + threadIndex, div.quot + 1) : JobRange(threadIndex * div.quot + div.rem, div.quot);
		}
	}
}