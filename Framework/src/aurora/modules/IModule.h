#pragma once

#include "aurora/SerializableObject.h"
#include "aurora/Ref.h"

namespace aurora::modules {
	enum class ModuleType : uint32_t {
		UNKNOWN = 0,
		GRAPHICS = 0b1,
		INPUT = 0b10
	};


	class AE_FW_DLL IModule : public Ref {
	public:
		virtual ~IModule() {}

		virtual ModuleType AE_CALL getType() const = 0;
	};
}