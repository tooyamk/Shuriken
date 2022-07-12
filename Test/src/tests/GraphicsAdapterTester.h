#pragma once

#include "../BaseTester.h"

class GraphicsAdapterTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		IntrusivePtr win = new Window();

		std::vector<GraphicsAdapter> gas;
		GraphicsAdapter::query(gas);

		for (auto& ga : gas) {
			printaln(ga.description);
			printaln("	vid = ", ga.vendorId);
			printaln("	did = ", ga.deviceId);
			printaln("	dedicatedVideoMemory = ", ga.dedicatedVideoMemory);
			printaln("	dedicatedSystemMemory = ", ga.dedicatedSystemMemory);
			printaln("	sharedSystemMemory = ", ga.sharedSystemMemory);
		}

		WindowStyle wndStype;
		wndStype.thickFrame = true;
		wndStype.backgroundColor.set(255, 255, 0);
		if (win->create(wndStype, "Fucker", Vec2ui32(800, 600), false)) {
			//app->setWindowPosition({200, 300});

			IntrusivePtr looper = new Looper(1.0 / 60.0);

			win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				//auto val = (bool*)e.getData();
				//*val = true;
			}));

			win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([win](Event<LooperEvent>& e) {
				win->pollEvents();
			}));

			win->setVisible(true);
			looper->run(true);
		}

		return 0;
	}
};