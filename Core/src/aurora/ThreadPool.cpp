#include "ThreadPool.h"

namespace aurora {
	ThreadPool::ThreadPool(size_t maxThreads) :
		_maxThreads(maxThreads),
		_isDestroyed(false) {
		
		_init();
	}

	ThreadPool::~ThreadPool() {
		destroy();
	}

	void ThreadPool::_init() {
		try {
			_allocateTasks();

			std::thread([this] {
				do {
					std::unique_lock<std::mutex> lock(_mutex);

					while (!_isDestroyed && _waitForRunningTasks.empty()) {
						_sync.wait(lock); //wait other trhead submit job
					}

					if (_isDestroyed) return;

					while (!_isDestroyed && _idleTasks.empty()) {
						_sync.wait(lock); // has task need run, but no have idle thread, wait other task complete
					}

					if (_isDestroyed) return;

					//has task, also has idle thread
					auto t = _getTask();
					auto job = _waitForRunningTasks.front();
					_waitForRunningTasks.pop_front();
					//t->testJob();
					t->setTask(shared_from_this(), job); //distribute tash to thread
				} while (true);
			}).detach();
		} catch (std::exception& e) {
			println("start threads pool error : ", e.what());
		}
	}

	void ThreadPool::_allocateTasks() {
		for (size_t i = 0; i < _maxThreads; ++i) {
			auto t = std::make_shared<Task>();
			try {
				t->start();
				_idleTasks.emplace_back(t);
			} catch (std::exception& e) { //exceed max thread num of process
				println("allocate threads error : ", e.what());
				break;
			}
		}
	}

	std::shared_ptr<ThreadPool::Task> ThreadPool::_getTask() {
		//get task object
		if (!_idleTasks.empty()) {
			auto t = *_idleTasks.begin();
			_idleTasks.pop_front();  //remove from idle queue
			_busyingTasks.emplace(t); //add to busy queue

			return t;
		}

		return std::shared_ptr<Task>();
	}

	void ThreadPool::destroy() {
		std::scoped_lock lock(_mutex);

		if (!_isDestroyed) {
			_isDestroyed = true;

			for (auto& t : _idleTasks) t->stop();
			_idleTasks.clear();

			for (auto& t : _busyingTasks) t->stop();

			_waitForRunningTasks.clear();

			_sync.notify_one();
		}
	}

	void ThreadPool::runTask(const std::function<void()>& task) {
		if (_isDestroyed) return;

		std::scoped_lock lock(_mutex);

		auto needNotify = _waitForRunningTasks.empty();
		_waitForRunningTasks.emplace_back(task);

		if (needNotify) _sync.notify_one(); //wait idle thread, need notify. other situations not required notify
	}

	void ThreadPool::_jobCompleted(const std::shared_ptr<ThreadPool::Task>& t) {
		std::scoped_lock lock(_mutex);

		auto needNotify = _idleTasks.empty() && (!_waitForRunningTasks.empty());
		_busyingTasks.erase(t);
		_idleTasks.emplace_back(t);

		if (needNotify) _sync.notify_one(); //task too more, no idle threads, wait notify
	}


	void ThreadPool::Task::start() {
		std::thread([this]() {
			while (!_exit) {
				std::unique_lock<std::mutex> lock(_mutex);

				///*
				if (_task) {//has task, need run
					try {
						_task(); //run task
					} catch (std::exception& e) {
						println("thread start error : ", e.what());
					} catch (...) {
						println("thread start error : unknown");
					}

					_task = nullptr;
					auto pool = _pool;
					_pool.reset();

					pool->_jobCompleted(shared_from_this()); //task complete, notify thread pool
				} else {
					_sync.wait(lock);//when no task, wait other thread submit task
				}
				//*/

				//_sync.wait(lock);
			}
		}).detach();
	}
}