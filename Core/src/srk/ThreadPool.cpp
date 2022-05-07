#include "ThreadPool.h"

namespace srk {
	ThreadPool::ThreadPool(size_t threadCount) :
		_stop(false),
		_activeThreads(0) {

		for (size_t i = 0; i < threadCount; ++i) {
			_workers.emplace_back([this] {
				do {
					PackagedTask task;

					{
						std::unique_lock<std::mutex> lock(_queueMutex);

						_queueCond.wait(lock, [this] { return _stop || !_tasks.empty(); });

						if (_stop && _tasks.empty()) return;

						++_activeThreads;
						task = std::move(_tasks.front());
						_tasks.pop_front();
					}

					task();

					bool notify;
					{
						std::lock_guard<std::mutex> lock(_queueMutex);
						--_activeThreads;
						notify = _workCompletedUnlocked();
					}
					if (notify) _completionCond.notify_all();
				} while (true);
			});
		}
	}

	ThreadPool::~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(_queueMutex);
			_stop = true;
		}

		_queueCond.notify_all();
		for (auto& worker : _workers) worker.join();
	}
}