#pragma once

#include "modules/Module.h"
#include "base/TSList.h"

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
		InputModule();
		virtual ~InputModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		inline bool AE_CALL isEnabled() const {
			return _isEnabled;
		}

		virtual void AE_CALL setEnabled(bool isEnabled) = 0;
		virtual void AE_CALL pollEvents() = 0;
		virtual events::IEventDispatcher<InputEvent>* AE_CALL getEventDispatcher() const = 0;

	protected:
		bool _isEnabled;
	};
}