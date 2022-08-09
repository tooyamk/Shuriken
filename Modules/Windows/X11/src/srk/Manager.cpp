#include "Manager.h"
#include "Window.h"
#include "CreateModule.h"

namespace srk::modules::windows::x11 {
	Manager::Manager(Ref* loader) :
		_loader(loader) {
	}

	Manager::~Manager() {
	}

	IntrusivePtr<IWindow> Manager::crerateWindow(const CreateWindowDesc& desc) {
		auto win = new Window(*this);
		if (!win->create(desc)) {
			delete win;
			win = nullptr;
		}

		return win;
	}

	bool Manager::processEvent(const IWindowModule::EventFn& fn) const {
		XEvent e = { 0 };
		if (!Window::checkIfEvent(&e)) return false;

		sendEvent((void*)e.xclient.window, &e, fn);
		return true;
	}
}