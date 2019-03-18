#include "Application.h"
#include "events/IEventDispatcher.h"
#include <thread>

namespace aurora {
	Application::Application(f64 frameInterval) :
		_eventDispatcher(nullptr),
		_time(0) {
		setFrameInterval(frameInterval);
	}

	Application::~Application() {
		AE_FREE_REF(_eventDispatcher);
	}

	void Application::setEventDispatcher(event::IEventDispatcher<Event>* eventDispatcher) {
		if (_eventDispatcher != eventDispatcher) AE_SET_REF(_eventDispatcher, eventDispatcher);
	}

	void Application::setFrameInterval(f64 frameInterval) {
		_frameInterval = frameInterval <= 0 ? 0 : frameInterval;
	}

	void Application::resetDeltaRecord() {
		_time = 0;
	}

	void Application::run() {
		MSG msg;
		memset(&msg, 0, sizeof(msg));

		while (msg.message != WM_QUIT) {
			if (PeekMessage(
				&msg,     // 存储消息的结构体指针
				nullptr,  // 窗口消息和线程消息都会被处理 
				0,        // 消息过滤最小值; 为0时返回所有可用信息
				0,        // 消息过滤最大值; 为0时返回所有可用信息
				PM_REMOVE // 指定消息如何处理; 消息在处理完后从队列中移除
			)) {
				TranslateMessage(&msg); // 变换虚拟键消息到字符消息，字符消息被发送到调用线程的消息队列
				DispatchMessage(&msg);  // 派发消息到窗口过程
			} else {
				//if (++aa == 120) {
					//PostQuitMessage(0);
				//}

				update(true);
			}
		}
	}

	void Application::update(bool autoSleep) {
		auto t0 = getTimeNow<std::chrono::microseconds, std::chrono::steady_clock>();
		f64 dt = _time == 0 ? 0 : (t0 - _time) * 0.001;//ms
		_time = t0;

		if (_eventDispatcher) _eventDispatcher->dispatchEvent(this, Event::ENTER_FRAME);

		if (autoSleep) {
			auto t1 = getTimeNow<std::chrono::microseconds, std::chrono::steady_clock>();

			f64 timePhase = f64(t1 - t0);
			if (timePhase < _frameInterval) std::this_thread::sleep_for(std::chrono::microseconds(i64(_frameInterval - timePhase)));
		}
	}

	void Application::shutdown() {
		PostQuitMessage(0);
	}
}