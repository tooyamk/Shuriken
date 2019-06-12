#pragma once

#include "base/Args.h"
#include "base/Ref.h"

namespace aurora::modules {
	enum class ModuleType : ui32 {
		UNKNOWN = 0,
		GRAPHICS = 0b1,
		INPUT = 0b10
	};


	class AE_DLL IModule : public Ref {
	public:
		virtual ~IModule() {}

		virtual ModuleType AE_CALL getType() const = 0;
	};
}