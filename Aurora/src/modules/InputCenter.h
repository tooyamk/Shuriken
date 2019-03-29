#pragma once

#include "modules/InputModule.h"
#include "events/IEventDispatcher.h"
#include <atomic>

namespace aurora::modules {
	class AE_DLL InputCenter : public InputModule {
	public:
		InputCenter(const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& eventDispatcherAllocator);
		virtual ~InputCenter();

		void AE_CALL addInput(InputModule* input);
		bool AE_CALL removeInput(InputModule* input);
		void AE_CALL removeAllInputs();
		ui32 AE_CALL getNumInputs() const;

		virtual void AE_CALL setEnabled(bool isEnabled) override;
		virtual void AE_CALL pollEvents() override;
		virtual InputModule::CREATE_PARAMS::EVENT_DISPATCHER* AE_CALL getEventDispatcher() const override;

	protected:
		std::atomic_bool _isDispatching;

		const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& _eventDispatcherAllocator;
		InputModule::CREATE_PARAMS::EVENT_DISPATCHER* _eventDispatcher;

		TSList<InputModule*> _inputs;

		struct Buf {
			i8 data[sizeof(InputEvent) + sizeof(InputKey)];
		};

		std::vector<Buf> _keysBuf1;
		std::vector<Buf> _keysBuf2;
		std::atomic<std::vector<Buf>*> _curKeysBuf;
		std::vector<Buf>* _waitKeysBuf;

		inline InputEvent& _getWritableEventType(Buf& buf) {
			return *(InputEvent*)buf.data;
		}

		inline InputKey& _getWritableKey(Buf& buf) {
			return *(InputKey*)(buf.data + sizeof(InputEvent));
		}

		inline void _writeKeyInfo(events::Event<InputEvent>& e) {
			auto& buf = _curKeysBuf->emplace_back();
			_getWritableEventType(buf) = e.getType();
			memcpy(&_getWritableKey(buf), e.getData(), sizeof(InputKey));
		}

		using Listener = events::EventListener<InputEvent, InputCenter>;

		Listener _keyListener;

		void _keyHandler(events::Event<InputEvent>& e);

		void _enableInput(InputModule* input);
		void _disableInput(InputModule* input);

		void _enableInputsTraverse(InputModule*& input);
		void _disableInputsTraverse(InputModule*& input);
		void _removeInputsTraverse(InputModule*& input);
	};
}