#include "InputModule.h"
#include "events/IEventDispatcher.h"

namespace aurora::modules {
	InputModule::~InputModule() {
	}


	InputCenter::InputCenter(const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& eventDispatcherAllocator) :
		_isEnabled(true),
		_eventDispatcherAllocator(eventDispatcherAllocator) {
		_eventDispatcher = _eventDispatcherAllocator.create();
	}

	InputCenter::~InputCenter() {
		setEnabled(false);
		_eventDispatcherAllocator.release(_eventDispatcher);
	}

	bool InputCenter::isEnabled() const {
		return _isEnabled;
	}

	void InputCenter::setEnabled(bool isEnabled) {
		if (_isEnabled != isEnabled) {
			_isEnabled = isEnabled;
		}
	}

	void InputCenter::pollEvents() {

	}

	InputModule::CREATE_PARAMS::EVENT_DISPATCHER* InputCenter::getEventDispatcher() const {
		return _eventDispatcher;
	}
}