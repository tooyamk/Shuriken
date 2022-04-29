#include "IShaderTranspiler.h"

namespace aurora::modules::graphics {
	ModuleType IShaderTranspiler::getType() const {
		return ModuleType::UNKNOWN;
	}
}