#pragma once

#include "modules/IModule.h"
#include "modules/graphics/ProgramSource.h"

namespace aurora::modules::graphics {
	class AE_DLL IProgramSourceTranslator : public IModule {
	public:
		virtual ModuleType AE_CALL getType() const override { return ModuleType::UNKNOWN; }
		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string& targetVersion) = 0;
	};
}