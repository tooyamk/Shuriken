#pragma once

#include "../BaseTester.h"
#include <shared_mutex>

class LockfreeTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		IntrusivePtr app = new Application("TestApp");

		ApplicationStyle wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, "", Vec2ui32(800, 600), false)) {
			IntrusivePtr looper = new Looper(1000.0 / 60.0);

			app->getEventDispatcher()->addEventListener(ApplicationEvent::CLOSED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([app](Event<LooperEvent>& e) {
				app->pollEvents();
			}));

			app->setVisible(true);

			_testQueue();

			looper->run(true);
		}

		return 0;
	}

private:
	void AE_CALL _testQueue() {
		using Class = lockfree::LinkedQueue<uint32_t, lockfree::QueueMode::MPMC>;
		auto queue = std::make_shared<Class>();
		auto tmpVec = std::make_shared<std::vector<uint32_t>>();
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
			std::thread([queue, tmpVec, tmpVecMtx, accumulative]() {
				do {
					uint32_t val;
					while (queue->pop(val)) {
						std::scoped_lock lock(*tmpVecMtx);
						tmpVec->emplace_back(val);
						if (val > 1000000) {
							int a = 1;
						}
					}

					{
						std::scoped_lock lock(*tmpVecMtx);
						if (*accumulative + tmpVec->size() == 1000000) break;
					}
					std::this_thread::sleep_for(1ms);
				} while (true);

				printaln("exit pop");
			}).detach();
		}

		std::thread([queue, tmpVec, tmpVecMtx, accumulative]() {
			do {
				auto old = *accumulative;

				{
					do {
						auto old1 = *accumulative;

						std::scoped_lock lock(*tmpVecMtx);
						if (!tmpVec->empty()) {
							auto next = *accumulative + 1;
							for (size_t i = 0, n = tmpVec->size(); i < n; ++i) {
								if (auto val = (*tmpVec)[i]; val == next) {
									++(*accumulative);
									tmpVec->erase(tmpVec->begin() + i);
									break;
								} else if (val <= *accumulative) {
									//error
									int a = 1;
								}
							}
						}

						if (old1 == *accumulative) break;
					} while (true);
				}

				if (old != *accumulative) {
					printaln("_accumulative : ", *accumulative);
				}

				std::this_thread::sleep_for(1ms);
			} while (true);

			printaln("exit print");
		}).detach();
	}
};