#include "Keyboard.h"
#include "base/Application.h"
#include "base/Time.h"
#include "events/IEventDispatcher.h"

namespace aurora::modules::input_win_kb {
	Keyboard::Keyboard(Keyboard::CREATE_PARAMS_REF params) :
		_isEnabled(true),
		_app(params.application->ref<Application>()),
		_eventDispatcherAllocator(*params.eventDispatcherAllocator),
		_downListener(this, &Keyboard::_downHandler),
		_upListener(this, &Keyboard::_upHandler) {
		_eventDispatcher = _eventDispatcherAllocator.create();
		auto appEvtDspt = _app->getEventDispatcher();
		appEvtDspt->addEventListener(ApplicationEvent::SYS_KEY_DOWN, _downListener, false);
		appEvtDspt->addEventListener(ApplicationEvent::SYS_KEY_UP, _upListener, false);
	}

	Keyboard::~Keyboard() {
		setEnabled(false);
		_eventDispatcherAllocator.release(_eventDispatcher);
		Ref::setNull(_app);
	}

	ui32 Keyboard::getType() const {
		return ModuleType::KEYBOARD | InputModule::getType();
	}

	bool Keyboard::isEnabled() const {
		return _isEnabled;
	}

	void Keyboard::setEnabled(bool isEnabled) {
		if (_isEnabled != isEnabled) {
			_isEnabled = isEnabled;

			auto appEvtDspt = _app->getEventDispatcher();
			if (_isEnabled) {
				appEvtDspt->addEventListener(ApplicationEvent::SYS_KEY_DOWN, _downListener, false);
				appEvtDspt->addEventListener(ApplicationEvent::SYS_KEY_UP, _upListener, false);
			} else {
				appEvtDspt->removeEventListener(ApplicationEvent::SYS_KEY_DOWN, _downListener);
				appEvtDspt->removeEventListener(ApplicationEvent::SYS_KEY_UP, _upListener);
				_keys.clear();
			}
		}
	}

	void Keyboard::pollEvents() {
		for (ui32 i = 0; i < _keys.size(); ++i) {
			auto& info = _keys[i];
			if (info.state) {
				InputKey key{info.code, 1.f, info.timestamp};
				_eventDispatcher->dispatchEvent(this, InputEvent::DOWN, &key);
			} else {
				InputKey key{ info.code, 0.f, info.timestamp };
				_eventDispatcher->dispatchEvent(this, InputEvent::UP, &key);
			}
		}
		_keys.clear();
	}

	InputModule::CREATE_PARAMS::EVENT_DISPATCHER* Keyboard::getEventDispatcher() const {
		return _eventDispatcher;
	}

	void Keyboard::_downHandler(events::Event<ApplicationEvent>& e) {
		auto msg = (MSG*)e.getData();

		auto& info = _keys.emplace_back();
		info.code = msg->wParam;
		info.state = 1;
		info.timestamp = Time::now();
	}

	void Keyboard::_upHandler(events::Event<ApplicationEvent>& e) {
		auto msg = (MSG*)e.getData();

		auto& info = _keys.emplace_back();
		info.code = msg->wParam;
		info.state = 0;
		info.timestamp = Time::now();
	}
}