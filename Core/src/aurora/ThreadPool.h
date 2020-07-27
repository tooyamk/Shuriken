#pragma once

#include "aurora/Global.h"
#include <functional>
#include <mutex>
#include <thread>
#include <memory>
#include <list>
#include <set>
#include <queue>
#include <condition_variable>

namespace aurora {
	class AE_CORE_DLL ThreadPool : public std::enable_shared_from_this<ThreadPool> {
	private:
		class Task : public std::enable_shared_from_this <Task> {
		public:
			Task() : _exit(false) {}
			Task(const Task&) = delete;
			~Task() {}
			Task& operator=(const Task&) = delete;

			void AE_CALL start();

			inline void AE_CALL stop() {
				_exit = true;
				_sync.notify_one();
			}

			inline void AE_CALL setTask(const std::shared_ptr<ThreadPool>& pool, const std::function<void()>& task) {//sumbit task
				std::unique_lock<std::mutex> lock(_mutex);

				_pool = pool;
				_task = task;
				_sync.notify_one();//notify main thread has task
			}

		private:
			bool _exit;
			std::mutex _mutex;
			std::condition_variable _sync;
			std::shared_ptr<ThreadPool> _pool;
			std::function<void()> _task;          //any time, only one task run
		};

	public:
		ThreadPool(size_t maxThreads);
		~ThreadPool();

		void AE_CALL runTask(const std::function<void()>& task);
		void AE_CALL destroy();

		inline bool AE_CALL isEmpty() const {
			std::unique_lock<std::mutex> lock(_mutex);
			return _idleTasks.size() == _maxThreads;
		}

		inline size_t AE_CALL getNumWaitingTasks() const {
			std::unique_lock<std::mutex> lock(_mutex);
			return _waitForRunningTasks.size();
		}

	private:
		bool _isDestroyed;
		size_t _maxThreads;
		std::list<std::shared_ptr<Task>> _idleTasks;
		std::set<std::shared_ptr<Task>> _busyingTasks;
		std::list<std::function<void()>> _waitForRunningTasks;
		mutable std::mutex _mutex;
		std::condition_variable _sync;

		void AE_CALL _init();
		std::shared_ptr<Task> AE_CALL _getTask();
		void AE_CALL _allocateTasks();
		void AE_CALL _jobCompleted(const std::shared_ptr<Task>& t);
	};
}