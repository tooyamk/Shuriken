#include "Manager.h"
#include "Window.h"
#include "CreateModule.h"

namespace srk::modules::windows::android_native {
	Manager::Manager(Ref* loader) :
		_loader(loader),
        _nativeWindow(nullptr),
        _evtHandler(&Manager::_recvEvent, this),
        _evtQueue(512) {
        _evtHandler.ref();
        extensions::AndroidNativeApplication::getInstance()->lockDo(this, [](extensions::AndroidNativeApplication& app, Manager* mgr){
            auto evtDispr = app.getEventDispatcher();
            evtDispr->addEventListener(extensions::AndroidNativeApplication::Event::STATE_CHANGED, mgr->_evtHandler);
            evtDispr->addEventListener(extensions::AndroidNativeApplication::Event::WINDOW_CHANGED, mgr->_evtHandler);
            evtDispr->addEventListener(extensions::AndroidNativeApplication::Event::FOCUS_CHANGED, mgr->_evtHandler);

            mgr->_nativeWindow = app.getWindow();
            mgr->_hasFocus = app.hasFocus();
        });
	}

	Manager::~Manager() {
        extensions::AndroidNativeApplication::getInstance()->lockDo(this, [](extensions::AndroidNativeApplication& app, Manager* mgr){
            auto evtDispr = app.getEventDispatcher();
            evtDispr->removeEventListener(extensions::AndroidNativeApplication::Event::STATE_CHANGED, mgr->_evtHandler);
            evtDispr->removeEventListener(extensions::AndroidNativeApplication::Event::WINDOW_CHANGED, mgr->_evtHandler);
            evtDispr->removeEventListener(extensions::AndroidNativeApplication::Event::FOCUS_CHANGED, mgr->_evtHandler);
        });
	}

	IntrusivePtr<IWindow> Manager::crerate(const CreateWindowDescriptor& desc) {
        if (!_windows.empty()) return nullptr;

		auto win = new Window(*this);
		if (!win->create(desc)) {
			delete win;
			win = nullptr;
		}
        
		return win;
	}

	bool Manager::processEvent(const IWindowModule::EventFn& fn) const {
        Message msg;
        if (!_evtQueue.pop(msg)) return false;

        switch (msg.type) {
		case extensions::AndroidNativeApplication::Event::WINDOW_CHANGED:
			_nativeWindow = msg.data.window;
			break;
        case extensions::AndroidNativeApplication::Event::FOCUS_CHANGED:
            _hasFocus = msg.data.hasFocus;
            break;
        }

        sendEvent(extensions::AndroidNativeApplication::getInstance()->getActivity(), &msg, fn);
        
        switch (msg.type) {
        case extensions::AndroidNativeApplication::Event::WINDOW_CHANGED:
        {
            {
                std::unique_lock lock(_mutex);

                _processedNativeWindow = msg.data.window;
                _cond.notify_all();
            }

            break;
        }
        }

		return true;
	}

    void Manager::_recvEvent(events::Event<extensions::AndroidNativeApplication::Event>& e) {
        switch (e.getType()) {
        case extensions::AndroidNativeApplication::Event::STATE_CHANGED:
        {
            Message msg;
            msg.type = extensions::AndroidNativeApplication::Event::STATE_CHANGED;
            msg.data.state = *e.getData<extensions::AndroidNativeApplication::State>();
            _evtQueue.push(msg);
            
            break;
        }
        case extensions::AndroidNativeApplication::Event::WINDOW_CHANGED:
        {
            auto window = e.getData<ANativeWindow>();

            Message msg;
            msg.type = extensions::AndroidNativeApplication::Event::WINDOW_CHANGED;
            msg.data.window = window;

            {
                std::unique_lock lock(_mutex);

                _evtQueue.push(msg);
                while (_processedNativeWindow != window) _cond.wait(lock);
            }
            
            break;
        }
        case extensions::AndroidNativeApplication::Event::FOCUS_CHANGED:
        {
            Message msg;
            msg.type = extensions::AndroidNativeApplication::Event::FOCUS_CHANGED;
            msg.data.hasFocus = *e.getData<bool>();
            _evtQueue.push(msg);

            break;
        }
        }
    }
}