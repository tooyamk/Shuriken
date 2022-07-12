#pragma once

#include "../BaseTester.h"

class WindowTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		IntrusivePtr app = new Application("TestApp");

		/*printaln("==>>>>", GetCurrentProcess(), "     ", GetCurrentProcessId());
		auto ppp = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
		TerminateProcess(ppp, 0);*/

		std::vector<IntrusivePtr<IWindow>> activedWindows;

		auto tryCreateWndFn = [&activedWindows, app](const WindowStyle& style, const std::string_view& title) {
			IntrusivePtr win = new Window();
			if (win->create(*app, style, title, Vec2ui32(800, 600), false)) {
				activedWindows.emplace_back(win);

				win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					//auto val = (bool*)e.getData();
					//*val = true;
				}));

				win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([&activedWindows, app](Event<WindowEvent>& e) {
					/*printaln("closed start");
					auto win = (IWindow*)e.getTarget();
					for (size_t i = 0, n = activedWindows.size(); i < n; ++i)
					{
						if (activedWindows[i] == win) {
							activedWindows.erase(activedWindows.begin() + i);
							break;
						}
					}
					printaln("closed end");

					if (activedWindows.empty()) {
						app->terminate();
					}*/
				}));

				win->getEventDispatcher()->addEventListener(WindowEvent::FOCUS_IN, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					auto win = (IWindow*)e.getTarget();
					printaln("wnd : ", win->getTitle(), " => focus in");
				}));

				win->getEventDispatcher()->addEventListener(WindowEvent::FOCUS_OUT, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					auto win = (IWindow*)e.getTarget();
					printaln("wnd : ", win->getTitle(), " => focus out");
				}));

				win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					auto win = (IWindow*)e.getTarget();
					auto size = win->getCurrentClientSize();
					printaln("wnd : ", win->getTitle(), " => resize  ", size[0], "   ", size[1]);
				}));
			}
		};

		WindowStyle wndStype;
		wndStype.thickFrame = true;
		wndStype.backgroundColor.set(255, 255, 0);
		tryCreateWndFn(wndStype, "Fucker1");

		wndStype.backgroundColor.set(255, 0, 0);
		tryCreateWndFn(wndStype, "Fucker2");
		if (!activedWindows.empty()) {
			//app->setWindowPosition({200, 300});

			IntrusivePtr looper = new Looper(1.0 / 60.0);

			auto t = srk::Time::now();
			int step = 0;

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([&activedWindows, &t, &step, looper](Event<LooperEvent>& e) {
				for (auto itr = activedWindows.begin(); itr != activedWindows.end();) {
					auto win = *itr;
					win->pollEvents();
					if (win->getApplication()) {
						++itr;
					} else {
						itr = activedWindows.erase(itr);
					}
				}

				if (activedWindows.empty()) looper->stop();

				return;
				//auto tt = srk::Time::now();
				//auto d = tt - t;
				//if (d >= 2000) {
				//	t = tt;
				//	if (step == 0) {
				//		step = 1;
				//		win->setMaximum();
				//		//app->toggleFullscreen();
				//		//app->setRestore();
				//		//app->getCurrentClientSize();
				//		//app->setClientSize(Vec2ui32(400, 400));
				//		//app->setVisible(false);
				//		//app->setWindowPosition(Vec2i32(800, 10));
				//		//app->setMaximum();
				//		//app->shutdown();
				//	} else if (step == 1) {
				//		step = 2;
				//		//app->toggleFullscreen();
				//		//app->setVisible(true);
				//	} else if (step == 2) {
				//		step = 3;
				//		//app->toggleFullscreen();
				//		//app->setVisible(true);
				//	}
				//}

				//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
			}));

			for (auto& win : activedWindows) win->setVisible(true);
			//app->setMaximum();
			//app->setMaximum();
			//app->setWindowPosition(Vec2i32(-400, 10));
			//app->setFocus();
			looper->run(true);
		}

		return 0;
	}
};