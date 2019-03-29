#include "InputCenter.h"

namespace aurora::modules {
	InputCenter::InputCenter(const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& eventDispatcherAllocator) :
		_isDispatching(false),
		_eventDispatcherAllocator(eventDispatcherAllocator),
		_keysBuf1(),
		_keysBuf2(),
		_curKeysBuf(&_keysBuf1),
		_waitKeysBuf(&_keysBuf2),
		_keyListener(this, &InputCenter::_keyHandler) {
		_eventDispatcher = _eventDispatcherAllocator.create();
	}

	InputCenter::~InputCenter() {
		removeAllInputs();
		_eventDispatcherAllocator.release(_eventDispatcher);
	}

	void InputCenter::addInput(InputModule* input) {
		if (input && _inputs.find(input) == _inputs.end()) {
			input->ref();
			_inputs.pushBack(input);

			if (_isEnabled) _enableInput(input);
		}
	}

	bool InputCenter::removeInput(InputModule* input) {
		if (!input) return false;

		auto itr = _inputs.find(input);
		if (itr == _inputs.end()) {
			return false;
		} else {
			_inputs.erase(itr);

			if (_isEnabled) _disableInput(input);
			input->unref();
			return true;
		}
	}

	void InputCenter::removeAllInputs() {
		if (_inputs.size()) {
			if (_isEnabled) {
				_inputs.beginTraverse();
				_inputs.traverse(this, &InputCenter::_removeInputsTraverse);
				_inputs.endTraverse();
			}

			_inputs.clear();
		}
	}

	ui32 InputCenter::getNumInputs() const {
		return _inputs.size();
	}

	void InputCenter::setEnabled(bool isEnabled) {
		if (_isEnabled != isEnabled) {
			_isEnabled = isEnabled;

			_inputs.beginTraverse();
			if (_isEnabled) {
				_inputs.traverse(this, &InputCenter::_enableInputsTraverse);
			} else {
				_inputs.traverse(this, &InputCenter::_disableInputsTraverse);
			}
			_inputs.endTraverse();
		}
	}

	void InputCenter::pollEvents() {
		if (_isDispatching.load(std::memory_order_acquire)) return;
		_isDispatching.store(true, std::memory_order_relaxed);

		//(int a = 1);
		while (_curKeysBuf.load(std::memory_order_relaxed)->size()) {
			auto tmp = _curKeysBuf;
			_curKeysBuf = _waitKeysBuf;
			_waitKeysBuf = tmp;

			//sort

			for (auto& buf : *_waitKeysBuf) {
				_eventDispatcher->dispatchEvent(this, _getWritableEventType(buf), &_getWritableKey(buf));
			}
			_waitKeysBuf->clear();
		}
		
		_isDispatching.store(false, std::memory_order_relaxed);
	}

	events::IEventDispatcher<InputEvent>* InputCenter::getEventDispatcher() const {
		return _eventDispatcher;
	}

	void InputCenter::_keyHandler(events::Event<InputEvent>& e) {
		_writeKeyInfo(e);
	}

	void InputCenter::_enableInput(InputModule* input) {
		auto ed = input->getEventDispatcher();
		ed->addEventListener(InputEvent::DOWN, _keyListener, false);
		ed->addEventListener(InputEvent::UP, _keyListener, false);
	}

	void InputCenter::_disableInput(InputModule* input) {
		auto ed = input->getEventDispatcher();
		ed->removeEventListener(InputEvent::DOWN, _keyListener);
		ed->removeEventListener(InputEvent::UP, _keyListener);
	}

	void InputCenter::_enableInputsTraverse(InputModule*& input) {
		_enableInput(input);
	}

	void InputCenter::_disableInputsTraverse(InputModule*& input) {
		_disableInput(input);
	}

	void InputCenter::_removeInputsTraverse(InputModule*& input) {
		if (_isEnabled) _disableInput(input);
		input->unref();
	}
}