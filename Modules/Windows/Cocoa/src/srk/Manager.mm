#include "Manager.h"
#include "Window.h"
#include "CreateModule.h"

namespace srk::modules::windows::cocoa {
	Manager::Manager(Ref* loader) :
		_loader(loader) {
	}

	Manager::~Manager() {
	}

	IntrusivePtr<IWindow> Manager::crerate(const CreateWindowDescriptor& desc) {
		auto win = new Window(*this);
		if (!win->create(desc)) {
			delete win;
			win = nullptr;
		}

		return win;
	}

	bool Manager::processEvent(const IWindowModule::EventFn& fn) const {
		auto e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:true];
        if (!e) return false;

        if (sendEvent(e.window, e, fn)) [NSApp sendEvent:e];
        [e release];
        return true;
	}
}