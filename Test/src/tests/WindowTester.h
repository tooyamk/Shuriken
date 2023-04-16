#pragma once

#include "../BaseTester.h"

class WindowTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		/*printaln("==>>>>", GetCurrentProcess(), "     ", GetCurrentProcessId());
		auto ppp = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
		TerminateProcess(ppp, 0);*/

		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		std::vector<IntrusivePtr<IWindow>> activedWindows;

		auto tryCreateWndFn = [wm, &activedWindows](const CreateWindowDescriptor& desc) {
			if (auto win = wm->crerate(desc); win) {
				auto border = win->getFrameExtents();
				printaln(L"border "sv, border[0], L" "sv, border[1], L" "sv, border[2], L" "sv, border[3]);
				activedWindows.emplace_back(win);

				win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					//auto val = (bool*)e.getData();
					//*val = true;
					}));

				win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([&activedWindows](Event<WindowEvent>& e) {
					/*printaln(L"closed start"sv);
					auto win = (IWindow*)e.getTarget();
					for (size_t i = 0, n = activedWindows.size(); i < n; ++i)
					{
						if (activedWindows[i] == win) {
							activedWindows.erase(activedWindows.begin() + i);
							break;
						}
					}
					printaln(L"closed end"sv);

					if (activedWindows.empty()) {
						app->terminate();
					}*/
					}));

				win->getEventDispatcher()->addEventListener(WindowEvent::FOCUS_IN, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					auto win = (IWindow*)e.getTarget();
					printaln(L"wnd : "sv, win->getTitle(), L" => focus in"sv);
					}));

				win->getEventDispatcher()->addEventListener(WindowEvent::FOCUS_OUT, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					auto win = (IWindow*)e.getTarget();
					printaln(L"wnd : "sv, win->getTitle(), L" => focus out"sv);
					}));

				win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
					auto win = (IWindow*)e.getTarget();
					auto size = win->getContentSize();
					printaln(L"wnd : "sv, win->getTitle(), L" => resize  "sv, size[0], L"   "sv, size[1]);
					}));

				win->setVisible(true);
			}
		};

		auto deb = Application::isDebuggerAttached() ? "debugger attached"sv : ""sv;
		auto title = "Fucker1 " + deb;;
		CreateWindowDescriptor desc;
		desc.style.resizable = true;
		desc.style.maximizable = true;
		desc.style.backgroundColor.set(255, 255, 0);
		desc.contentSize.set(800, 600);
		desc.title = title;
			
		tryCreateWndFn(desc);

		title = "Fucker2 " + deb;;
		desc.style.backgroundColor.set(255, 0, 0);
		desc.title = title;
		tryCreateWndFn(desc);
		if (!activedWindows.empty()) {
			//app->setWindowPosition({200, 300});

			IntrusivePtr looper = new Looper(1.0 / 60.0);

			auto t = srk::Time::now();
			int step = 0;

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([wm, &activedWindows, &t, &step](Event<LooperEvent>& e) {
				while (wm->processEvent()) {};

				for (auto itr = activedWindows.begin(); itr != activedWindows.end();) {
					auto win = *itr;
					if (win->isClosed()) {
						itr = activedWindows.erase(itr);
					} else {
						++itr;
					}
				}

				if (activedWindows.empty()) {
					e.getTarget<Looper>()->stop();
					return;
				}

				//return;
				auto tt = srk::Time::now();
				auto d = tt - t;
				if (d >= 2000) {
					t = tt;
					if (step == 0) {
						step = 1;
						//activedWindows[0]->toggleFullScreen();
						//printaln(L"is visible "sv, activedWindows[0]->isVisible());
						//activedWindows[0]->toggleFullScreen();
						//app->toggleFullscreen();
						//app->setRestore();
						//app->getCurrentClientSize();
						//app->setClientSize(Vec2ui32(400, 400)); 
						//app->setVisible(false);
						//app->setWindowPosition(Vec2i32(800, 10));
						//app->setMaximum();
						//app->shutdown();
					} else if (step == 1) {
						//activedWindows[0]->setMinimum();
						//printaln(L"is min "sv, activedWindows[0]->isMinimzed());
						//printaln(L"is visible "sv, activedWindows[0]->isVisible());
						step = 2;
						//activedWindows[0]->setMaximum();
						//app->toggleFullscreen();
						//app->setVisible(true);
					} else if (step == 2) {
						//printaln(L"is visible "sv, activedWindows[0]->isVisible());
						step = 3;
						//app->toggleFullscreen();
						//app->setVisible(true);
					}
				}

				//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
				}));

			//for (auto& win : activedWindows) win->setVisible(true);
			//for (auto& win : activedWindows) win->setVisible(false);
			//app->setMaximum();
			//app->setMaximum();
			//app->setWindowPosition(Vec2i32(-400, 10));
			//app->setFocus();
			looper->run(true);
		}

		return 0;
	}
};