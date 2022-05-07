#include "IShaderTranspiler.h"

namespace srk::modules::graphics {
	ModuleType IShaderTranspiler::getType() const {
		return ModuleType::UNKNOWN;
	}
}