#pragma once

#include "modules/Module.h"

namespace aurora::modules {
	class AE_DLL InputModule : public Module {
	public:
		enum class Event : ui8 {
			DOWN,
			UP
		};


		using EVENT_DISPATCHER_ALLOCATOR = const events::IEventDispatcherAllocator<Event>&;
		using CREATE_MODULE_FN = InputModule*(*)(EVENT_DISPATCHER_ALLOCATOR);


		virtual ~InputModule() {}
	};
}