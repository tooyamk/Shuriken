#pragma once

#include "aurora/Global.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

namespace aurora {
	class AE_CORE_DLL ThreadPool {
	public:
		using PackagedTask = std::packaged_task<void()>;

		ThreadPool(size_t threadCount);
		~ThreadPool();

		template<typename F, typename... Args>
		std::shared_future<void> AE_CALL enqueue(F&& f, Args&&... args) {
			PackagedTask task(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

			std::future<void> future = task.get_future();
			{
				std::unique_lock<std::mutex> lock(_queueMutex);

				if (_stop) return std::future<void>().share();

				_tasks.emplace_back(std::move(task));
			}
			_queueCond.notify_one();
			return future.share();
		}

		inline void AE_CALL wait() {
			std::unique_lock<std::mutex> lock(_queueMutex);
			_completionCond.wait(lock, [&] { return _workCompletedUnlocked(); });
		}
		
	private:
		bool _stop;
		size_t _activeThreads;
		std::vector<std::thread> _workers;
		std::list<PackagedTask> _tasks;
		std::mutex _queueMutex;
		std::condition_variable _queueCond;
		std::condition_variable _completionCond;

		inline bool AE_CALL _workCompletedUnlocked() {
			return !_activeThreads && _tasks.empty();
		}
	};
}