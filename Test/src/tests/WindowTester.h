#pragma once

#include "../BaseTester.h"

class WindowTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		RefPtr app = new Application("TestApp");

		ApplicationStyle wndStype;
		wndStype.thickFrame = true;
		wndStype.backgroundColor.set(255, 255, 0);
		if (app->createWindow(wndStype, "Fucker", Vec2ui32(800, 600), false)) {
			//app->setWindowPosition({200, 300});

			RefPtr looper = new Looper(1000.0 / 60.0);

			auto t = aurora::Time::now();
			int step = 0;

			app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSING, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				//auto val = (bool*)e.getData();
				//*val = true;
			}));

			app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				looper->stop();
			}));

			app->getEventDispatcher().addEventListener(ApplicationEvent::FOCUS_IN, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				printaln("focus in");
			}));

			app->getEventDispatcher().addEventListener(ApplicationEvent::FOCUS_OUT, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				printaln("focus out");
			}));

			app->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				auto app = (IApplication*)e.getTarget();
				auto size = app->getCurrentClientSize();
				printaln("resize  ", size[0], "   ", size[1]);
			}));

			looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([app, &t, &step](Event<LooperEvent>& e) {
				app->pollEvents();

				auto tt = aurora::Time::now();
				auto d = tt - t;
				if (d >= 2000) {
					t = tt;
					if (step == 0) {
						step = 1;
						app->setMaximum();
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

			app->setVisible(true);
			//app->setMaximum();
			//app->setMaximum();
			//app->setWindowPosition(Vec2i32(-400, 10));
			//app->setFocus();
			looper->run(true);
		}

		return 0;
	}
};