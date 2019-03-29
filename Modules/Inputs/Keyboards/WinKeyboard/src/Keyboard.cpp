#include "Keyboard.h"
#include "base/Application.h"
#include "base/Time.h"
#include "events/IEventDispatcher.h"

namespace aurora::modules::input_win_kb {
	Keyboard::Keyboard(Keyboard::CREATE_PARAMS_REF params) :
		_isDispatching(false),
		_app(params.application->ref<Application>()),
		_eventDispatcherAllocator(*params.eventDispatcherAllocator),
		_downListener(this, &Keyboard::_downHandler),
		_upListener(this, &Keyboard::_upHandler) {
		_eventDispatcher = _eventDispatcherAllocator.create();
		auto appEvtDspt = _app->getEventDispatcher();
	}

	Keyboard::~Keyboard() {
		setEnabled(false);
		_eventDispatcherAllocator.release(_eventDispatcher);
		Ref::setNull(_app);
	}

	ui32 Keyboard::getType() const {
		return ModuleType::KEYBOARD | InputModule::getType();
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
		if (_isDispatching) return;
		_isDispatching = true;

		for (ui32 i = 0; i < _keys.size(); ++i) {
			auto& buf = _keys[i];
			_eventDispatcher->dispatchEvent(this, _getWritableEventType(buf), &_getWritableKey(buf));
		}

		_keys.clear();
		_isDispatching = false;
	}

	InputModule::CREATE_PARAMS::EVENT_DISPATCHER* Keyboard::getEventDispatcher() const {
		return _eventDispatcher;
	}

	void Keyboard::_downHandler(events::Event<ApplicationEvent>& e) {
		_writeKeyInfo(InputEvent::DOWN, *((MSG*)e.getData()), 0.f);
	}

	void Keyboard::_upHandler(events::Event<ApplicationEvent>& e) {
		_writeKeyInfo(InputEvent::UP, *((MSG*)e.getData()), 0.f);
	}

	void Keyboard::_writeKeyInfo(InputEvent type, const MSG& msg, f32 value) {
		auto& buf = _keys.emplace_back();

		_getWritableEventType(buf) = type;

		auto& key = _getWritableKey(buf);
		key.code = msg.wParam;
		key.value = value;
		key.timestamp = Time::now();
	}
}