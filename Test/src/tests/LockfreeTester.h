#pragma once

#include "../BaseTester.h"
#include <shared_mutex>

template<typename T>
class LockFreeSinglyLinkedList {
private:
	struct Node {
		volatile std::atomic<Node*> next;
		T item;

		Node() :
			next(nullptr) {
		}

		explicit Node(const T& item) :
			next(nullptr),
			item(item) {
		}

		explicit Node(T&& item) :
			next(nullptr),
			item(std::move(item)) {
		}
	};

	Node* _head;

	template<typename I>
	void _insertHead(I&& item) {
		auto n = new Node(std::forward<I>(item));

		do {
			auto next = _head->next.load(std::memory_order::acquire);
			n->next.store(next, std::memory_order::release);
			if (_head->next.compare_exchange_strong(next, n, std::memory_order::release, std::memory_order::relaxed)) break;
		} while (true);
	}

public:
	LockFreeSinglyLinkedList() :
		_head(new Node()) {
	}

	inline void insertHead(const T& item) {
		_insertHead(item);
	}

	inline void insertHead(T&& item) {
		_insertHead(item);
	}
};

class LockfreeTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		CreateWindowDescriptor desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerate(desc);
		if (!win) return 0;

		IntrusivePtr looper = new Looper(1.0 / 60.0);

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
			looper->stop();
			}));

		looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([wm](Event<LooperEvent>& e) {
			while (wm->processEvent()) {};
			}));

		win->setVisible(true);

		_testQueue();

		looper->run(true);

		return 0;
	}

private:
	void SRK_CALL _testQueue() {
		constexpr size_t max = 1000000;

		//using Class = lockfree::LinkedQueue<uint32_t, lockfree::LinkedQueueMode::MPMC>;
		using Class = lockfree::RingQueue<uint32_t, lockfree::RingQueueMode::SPSC>;
		auto queue = std::make_shared<Class>(max);
		auto tmpSet = std::make_shared<std::unordered_set<uint32_t>>();
		auto tmpVecMtx = std::make_shared<std::mutex>();
		auto idGenerator = std::make_shared<std::atomic_uint32_t>(0);
		auto accumulative = std::make_shared<std::uint32_t>(0);

		for (auto j = 0; j < (((uint8_t)(Class::MODE) & 0b10) ? 10 : 1); ++j) {
			std::thread([queue, idGenerator]() {
				do {
					auto id = idGenerator->fetch_add(1);
					if (id <= (max - 1)) {
						queue->push(id + 1);
					} else {
						break;
					}
				} while (true);

				printaln(L"exit push"sv);
			}).detach();
		}

		for (auto j = 0; j < (((uint8_t)(Class::MODE) & 0b1) ? 10 : 1); ++j) {
			std::thread([queue, tmpSet, tmpVecMtx, accumulative]() {
				do {
					uint32_t val;
					while (queue->pop(val)) {
						std::scoped_lock lock(*tmpVecMtx);
						tmpSet->emplace(val);
						if (val > max) {
							int a = 1;
						}
					}

					{
						std::scoped_lock lock(*tmpVecMtx);
						if (*accumulative + tmpSet->size() == max) break;
					}
					std::this_thread::sleep_for(1ms);
				} while (true);

				printaln(L"exit pop"sv);
			}).detach();
		}

		std::thread([queue, tmpSet, tmpVecMtx, accumulative]() {
			auto t0 = srk::Time::now();

			do {
				auto old = *accumulative;

				{
					do {
						auto old1 = *accumulative;

						std::scoped_lock lock(*tmpVecMtx);
						if (!tmpSet->empty()) {
							auto next = *accumulative + 1;
							auto itr = tmpSet->find(next);
							if (itr != tmpSet->end()) {
								++(*accumulative);
								tmpSet->erase(itr);
							}
						}

						if (old1 == *accumulative) break;
					} while (true);
				}

				auto t1 = srk::Time::now();
				auto forcePrint = t1 - t0 >= 2000;

				if ((old != *accumulative) || forcePrint) {
					printaln(L"_accumulative : "sv, *accumulative);
				}

				if (forcePrint) t0 = t1;

				std::this_thread::sleep_for(1ms);
			} while (true);

			printaln(L"exit print"sv);
		}).detach();
	}
};