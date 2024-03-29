#pragma once

#include "../BaseTester.h"

class GraphicsAdapterTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		std::vector<GraphicsAdapter> gas;
		GraphicsAdapter::query(gas);

		for (auto& ga : gas) {
			printaln(ga.description);
			printaln(L"	vid = "sv, ga.vendorId);
			printaln(L"	did = "sv, ga.deviceId);
			printaln(L"	dedicatedVideoMemory = "sv, ga.dedicatedVideoMemory);
			printaln(L"	dedicatedSystemMemory = "sv, ga.dedicatedSystemMemory);
			printaln(L"	sharedSystemMemory = "sv, ga.sharedSystemMemory);
		}

		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		CreateWindowDescriptor desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerate(desc);
		if (!win) return 0;

		IntrusivePtr looper = new Looper(1.0 / 60.0);

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
			//auto val = (bool*)e.getData();
			//*val = true;
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
			looper->stop();
			}));

		looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([wm](Event<LooperEvent>& e) {
			while (wm->processEvent()) {};
			}));

		win->setVisible(true);
		looper->run(true);

		return 0;
	}
};