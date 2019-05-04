#include "IProgramSourceTranslator.h"

namespace aurora::modules::graphics {
	ModuleType IProgramSourceTranslator::getType() const {
		return ModuleType::UNKNOWN;
	}
}