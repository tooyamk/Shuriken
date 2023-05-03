#pragma once

#include "srk/Core.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace srk {
	class SRK_CORE_DLL ThreadPool {
	public:
		using PackagedTask = std::packaged_task<void()>;

	private:
		struct TaskNode {
			TaskNode(PackagedTask&& task) : task(std::move(task)), next(nullptr) {}

			PackagedTask task;
			TaskNode* next;
		};

	public:

		ThreadPool(size_t threadCount);~ThreadPool();

		template<typename F, typename... Args>
		std::shared_future<void> SRK_CALL enqueue(F&& f, Args&&... args) {
			auto node = new TaskNode(PackagedTask([f = std::forward<F>(f), args...]() mutable { f(std::forward<Args>(args)...); }));

			std::future<void> future = node->task.get_future();
			{
				std::unique_lock<std::mutex> lock(_queueMutex);

				if (_stop) {
					delete node;
					return std::future<void>().share();
				}

				if (_taskTail) {
					_taskTail->next = node;
					_taskTail = node;
				} else {
					_taskHead = _taskTail = node;
				}
			}
			_queueCond.notify_one();
			return future.share();
		}

		inline void SRK_CALL wait() {
			std::unique_lock<std::mutex> lock(_queueMutex);
			_completionCond.wait(lock, [&] { return _workCompletedUnlocked(); });
		}
		
	private:
		bool _stop;
		size_t _activeThreads;
		std::vector<std::thread> _workers;
		TaskNode* _taskHead;
		TaskNode* _taskTail;
		std::mutex _queueMutex;
		std::condition_variable _queueCond;
		std::condition_variable _completionCond;

		inline bool SRK_CALL _workCompletedUnlocked() const {
			return !_activeThreads && !_taskHead;
		}
	};
}