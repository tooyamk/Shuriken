#include "ThreadPool.h"

namespace srk {
	ThreadPool::ThreadPool(size_t threadCount) :
		_stop(false),
		_taskHead(nullptr),
		_taskTail(nullptr),
		_activeThreads(0) {

		_workers.reserve(threadCount);
		for (size_t i = 0; i < threadCount; ++i) {
			_workers.emplace_back([this] {
				do {
					TaskNode* node;

					{
						std::unique_lock<std::mutex> lock(_queueMutex);

						_queueCond.wait(lock, [this] { return _stop || _taskHead; });

						if (_stop && !_taskHead) return;

						++_activeThreads;

						node = _taskHead;
						if (_taskHead->next) {
							_taskHead = _taskHead->next;
						} else {
							_taskHead = _taskTail = nullptr;
						}
					}

					node->task();
					delete node;

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