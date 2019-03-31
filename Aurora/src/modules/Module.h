#pragma once

#include "base/Ref.h"

namespace aurora {
	class Application;

	namespace events {
		template<typename EvtType> class IEventDispatcher;
	}
}

namespace aurora::modules {
	class AE_DLL ModuleType {
	public:
		ModuleType() = delete;
		ModuleType(const ModuleType&) = delete;
		ModuleType(ModuleType&&) = delete;

		static const ui32 GRAPHICS = 0b1;
		static const ui32 INPUT = 0b10;
		static const ui32 KEYBOARD = 0b100;
		static const ui32 MOUSE = 0b1000;
	};


	template<typename EvtType>
	class AE_TEMPLATE_DLL ModuleCreateParams {
	public:
		Application* application = nullptr;
	};


	template<typename EvtType>
	class AE_TEMPLATE_DLL Module : public Ref {
	public:
		using EVENT = EvtType;
		using CREATE_PARAMS = ModuleCreateParams<EVENT>;
		using CREATE_PARAMS_REF = const ModuleCreateParams<EVENT>&;
		using CREATE_PARAMS_PTR = const ModuleCreateParams<EVENT>*;

		virtual ~Module() {}

		virtual ui32 AE_CALL getType() const = 0;
	};
}