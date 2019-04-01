#pragma once

#include "base/Ref.h"

namespace aurora {
	class Application;
}

namespace aurora::modules {
	class AE_DLL ModuleType {
	public:
		ModuleType() = delete;
		ModuleType(const ModuleType&) = delete;
		ModuleType(ModuleType&&) = delete;

		static const ui32 GRAPHICS = 0b1;
		static const ui32 INPUT = 0b10;
	};


	class AE_DLL Module : public Ref {
	public:
		virtual ~Module() {}

		virtual ui32 AE_CALL getType() const = 0;
	};
}