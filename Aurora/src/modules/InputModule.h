#pragma once

#include "modules/Module.h"

namespace aurora::modules {
	enum class InputEvent : ui8 {
		DOWN,
		UP
	};


	class AE_DLL InputKey {
	public:
		i32 code;
		f32 value;
		i64 timestamp;
	};


	class AE_DLL InputModule : public Module<InputEvent> {
	public:
		virtual ~InputModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual bool AE_CALL isEnabled() const = 0;
		virtual void AE_CALL setEnabled(bool isEnabled) = 0;
		virtual void AE_CALL pollEvents() = 0;
		virtual InputModule::CREATE_PARAMS::EVENT_DISPATCHER* AE_CALL getEventDispatcher() const = 0;
	};


	class AE_DLL InputCenter : public InputModule {
	public:
		InputCenter(const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& eventDispatcherAllocator);
		virtual ~InputCenter();

		//void AE_CALL addInput(InputModule& input);

		virtual bool AE_CALL isEnabled() const override;
		virtual void AE_CALL setEnabled(bool isEnabled) override;
		virtual void AE_CALL pollEvents() override;
		virtual InputModule::CREATE_PARAMS::EVENT_DISPATCHER* AE_CALL getEventDispatcher() const override;

	protected:
		bool _isEnabled;

		const InputModule::CREATE_PARAMS::EVENT_DISPATCHER_ALLOCATOR& _eventDispatcherAllocator;
		InputModule::CREATE_PARAMS::EVENT_DISPATCHER* _eventDispatcher;
	};
}