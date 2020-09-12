#pragma once

#include "../BaseTester.h"

class WindowTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		RefPtr app = new Application("TestApp");

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, "", Box2i32ui32(Vec2i32(100, 100), Vec2ui32(800, 600)), false)) {
			Args args;
			args.add("app", &*app);

			RefPtr looper = new Looper(1000.0 / 60.0);

			auto t = aurora::Time::now();
			int step = 0;

			app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([app, &t, &step](Event<LooperEvent>& e) {
				app->pollEvents();

				auto tt = aurora::Time::now();
				auto d = tt - t;
				if (d >= 2000) {
					t = tt;
					if (step == 0) {
						step = 1;
						app->getClientSize();
						//app->setVisible(false);
					} else if (step == 1) {
						step = 2;
						//app->setVisible(true);
					}
				}

				//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
			}));

			//evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

			app->setVisible(true);
			looper->run(true);
		}

		return 0;
	}
};