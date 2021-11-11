#pragma once

#include "../BaseTester.h"

class GraphicsAdapterTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		IntrusivePtr app = new Application("TestApp");

		std::vector<GraphicsAdapter> gas;
		GraphicsAdapter::query(gas);

		for (auto& ga : gas) {
			printdln(ga.description);
			printdln("	vid = ", ga.vendorId);
			printdln("	did = ", ga.deviceId);
			printdln("	dedicatedVideoMemory = ", ga.dedicatedVideoMemory);
			printdln("	dedicatedSystemMemory = ", ga.dedicatedSystemMemory);
			printdln("	sharedSystemMemory = ", ga.sharedSystemMemory);
		}

		ApplicationStyle wndStype;
		wndStype.thickFrame = true;
		wndStype.backgroundColor.set(255, 255, 0);
		if (app->createWindow(wndStype, "Fucker", Vec2ui32(800, 600), false)) {
			//app->setWindowPosition({200, 300});

			IntrusivePtr looper = new Looper(1000.0 / 60.0);

			app->getEventDispatcher()->addEventListener(ApplicationEvent::CLOSING, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				//auto val = (bool*)e.getData();
				//*val = true;
			}));

			app->getEventDispatcher()->addEventListener(ApplicationEvent::CLOSED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([app](Event<LooperEvent>& e) {
				app->pollEvents();
			}));

			app->setVisible(true);
			looper->run(true);
		}

		return 0;
	}
};