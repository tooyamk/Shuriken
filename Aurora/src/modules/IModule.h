#pragma once

#include "base/Args.h"
#include "base/Ref.h"

namespace aurora::modules {
	class AE_DLL ModuleType {
	public:
		AE_DECLA_CANNOT_INSTANTIATE(ModuleType);

		static const ui32 GRAPHICS = 0b1;
		static const ui32 INPUT = 0b10;
	};


	class AE_DLL IModule : public Ref {
	public:
		virtual ~IModule() {}

		virtual ui32 AE_CALL getType() const = 0;
	};
}