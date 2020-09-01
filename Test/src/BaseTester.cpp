#include "BaseTester.h"

void Stats::run(Looper* looper) {
	if (looper) {
		looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, new EventListener(std::function([this](Event<LooperEvent>& e) {
			++_frameCount;
		})));

		std::thread([this, looper]() {
			auto tw = std::make_shared<TimeWheel>(100, 100);

			auto frameTime = Time::now();
			TimeWheel::Timer timer;
			timer.onTick = [this, looper, &frameTime](TimeWheel::Timer& timer) {
				auto t = Time::now();

				auto fps = _frameCount / ((t - frameTime) * 0.001);
				println("fps : ", fps);

				_frameCount = 0;
				frameTime = t;
			};
			tw->startTimer(timer, 1000000, 0, false);

			auto t0 = Time::now<std::chrono::microseconds>();
			while (true) {
				auto t = Time::now<std::chrono::microseconds>();
				uint64_t d = t - t0;
				while (d) {
					uint64_t e;
					if (d > tw->getInterval()) {
						e = tw->getInterval();
						d -= tw->getInterval();
					} else {
						e = d;
						d = 0;
					}

					tw->tick(e, [](TimeWheel::Timer& timer, uint64_t tickID) {
						timer.doTick(tickID);
					});
				}
				t0 = t;

				std::this_thread::sleep_for(1000us);
			}
		}).detach();
	}
}


int32_t BaseTester::run() {
	return 0;
}