#include "Manager.h"
#include "Window.h"
#include "CreateModule.h"

namespace srk::modules::windows::windows_classic {
	Manager::Manager(Ref* loader) :
		_loader(loader) {
	}

	Manager::~Manager() {
	}

	IntrusivePtr<IWindow> Manager::crerateWindow(const CreateWindowDesc& desc) {
		auto win = new Window();
		if (!win->create(*this, desc)) {
			delete win;
			win = nullptr;
		}

		return win;
	}

	bool Manager::processEvent(const IWindowModule::EventFn& fn) const {
		MSG msg = { 0 };
		if (!PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) return false;

		if (sendEvent(msg.hwnd, &msg, fn)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return true;
	}
}