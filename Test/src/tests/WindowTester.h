#pragma once

#include "../BaseTester.h"

class WindowTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		IntrusivePtr app = new Application("TestApp");
		IntrusivePtr win = new Window();
		IntrusivePtr win2 = new Window();

		/*printaln("==>>>>", GetCurrentProcess(), "     ", GetCurrentProcessId());
		auto ppp = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
		TerminateProcess(ppp, 0);*/

		WindowStyle wndStype;
		wndStype.thickFrame = true;
		wndStype.backgroundColor.set(255, 255, 0);
		if (win->create(*app, wndStype, "Fucker", Vec2ui32(800, 600), false)) {
			wndStype.backgroundColor.set(255, 0, 0);
			win2->create(*app, wndStype, "Fucker2", Vec2ui32(800, 600), false);
			//app->setWindowPosition({200, 300});

			IntrusivePtr looper = new Looper(1.0 / 60.0);

			auto t = srk::Time::now();
			int step = 0;

			win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				//auto val = (bool*)e.getData();
				//*val = true;
			}));

			win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				looper->stop();
			}));

			win->getEventDispatcher()->addEventListener(WindowEvent::FOCUS_IN, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				printaln("focus in");
			}));

			win->getEventDispatcher()->addEventListener(WindowEvent::FOCUS_OUT, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				printaln("focus out");
			}));

			win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				auto win = (IWindow*)e.getTarget();
				auto size = win->getCurrentClientSize();
				printaln("resize  ", size[0], "   ", size[1]);
			}));

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([win, win2, &t, &step](Event<LooperEvent>& e) {
				win->pollEvents();
				win2->pollEvents();

				return;
				auto tt = srk::Time::now();
				auto d = tt - t;
				if (d >= 2000) {
					t = tt;
					if (step == 0) {
						step = 1;
						win->setMaximum();
						//app->toggleFullscreen();
						//app->setRestore();
						//app->getCurrentClientSize();
						//app->setClientSize(Vec2ui32(400, 400));
						//app->setVisible(false);
						//app->setWindowPosition(Vec2i32(800, 10));
						//app->setMaximum();
						//app->shutdown();
					} else if (step == 1) {
						step = 2;
						//app->toggleFullscreen();
						//app->setVisible(true);
					} else if (step == 2) {
						step = 3;
						//app->toggleFullscreen();
						//app->setVisible(true);
					}
				}

				//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
			}));

			win->setVisible(true);
			win2->setVisible(true);
			//app->setMaximum();
			//app->setMaximum();
			//app->setWindowPosition(Vec2i32(-400, 10));
			//app->setFocus();
			looper->run(true);
		}

		return 0;
	}
};