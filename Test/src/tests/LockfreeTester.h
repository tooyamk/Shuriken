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
		IntrusivePtr app = new Application("TestApp");
		IntrusivePtr win = new Window();

		printFloat(0.0f);
		printFloat(0.1f);
		printFloat(0.2f);
		printFloat(0.3f);
		printFloat(0.4f);
		printFloat(0.5f);
		printFloat(0.6f);
		printFloat(0.7f);
		printFloat(0.8f);
		printFloat(0.9f);
		printFloat(1.0f);
		printFloat(0.123f);
		printFloat(0.125f);
		printFloat(0.111111111f);

		WindowStyle wndStype;
		wndStype.thickFrame = true;
		if (win->create(*app, wndStype, "", Vec2ui32(800, 600), false)) {
			IntrusivePtr looper = new Looper(1000.0 / 60.0);

			win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([win](Event<LooperEvent>& e) {
				win->pollEvents();
			}));

			win->setVisible(true);

			//_testQueue();

			looper->run(true);
		}

		return 0;
	}

	void printFloat(float32_t f) {
		auto iv = *(uint32_t*)&f;

		std::string s = String::toString(iv >> 31) + " ";
		{
			uint8_t val = iv >> 23 & 0xFF;
			val -= 127;
			//s += String::toString(val, 2);
			for (auto i = 0; i < 8; ++i) {
				s += String::toString(val >> (7 - i) & 0b1);
			}
		}
		s += " ";
		for (auto i = 0; i < 23; ++i) {
			s += String::toString(iv >> (22 - i) & 0b1);
		}

		printaln(f, "   ", s);
	}

private:
	void SRK_CALL _testQueue() {
		using Class = lockfree::LinkedQueue<uint32_t, lockfree::QueueMode::MPMC>;
		auto queue = std::make_shared<Class>();
		auto tmpSet = std::make_shared<std::unordered_set<uint32_t>>();
		auto tmpVecMtx = std::make_shared<std::mutex>();
		auto idGenerator = std::make_shared<std::atomic_uint32_t>(0);
		auto accumulative = std::make_shared<std::uint32_t>(0);

		for (auto j = 0; j < (((uint8_t)(Class::MODE) & 0b10) ? 10 : 1); ++j) {
			std::thread([queue, idGenerator]() {
				do {
					auto id = idGenerator->fetch_add(1);
					if (id <= 999999) {
						queue->push(id + 1);
					} else {
						break;
					}
				} while (true);

				printaln("exit push");
			}).detach();
		}

		for (auto j = 0; j < (((uint8_t)(Class::MODE) & 0b1) ? 10 : 1); ++j) {
			std::thread([queue, tmpSet, tmpVecMtx, accumulative]() {
				do {
					uint32_t val;
					while (queue->pop(val)) {
						std::scoped_lock lock(*tmpVecMtx);
						tmpSet->emplace(val);
						if (val > 1000000) {
							int a = 1;
						}
					}

					{
						std::scoped_lock lock(*tmpVecMtx);
						if (*accumulative + tmpSet->size() == 1000000) break;
					}
					std::this_thread::sleep_for(1ms);
				} while (true);

				printaln("exit pop");
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
					printaln("_accumulative : ", *accumulative);
				}

				if (forcePrint) t0 = t1;

				std::this_thread::sleep_for(1ms);
			} while (true);

			printaln("exit print");
		}).detach();
	}
};