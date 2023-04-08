#pragma once

#include "srk/Intrusive.h"

namespace srk::modules {
	enum class ModuleType : uint32_t {
		UNKNOWN = 0,
		GRAPHICS = 0b1,
		INPUT = 0b10,
		WINDOW = 0b100
	};


	class SRK_FW_DLL IModule : public Ref {
	public:
		virtual ~IModule() {}

		virtual ModuleType SRK_CALL getType() const = 0;
	};
}