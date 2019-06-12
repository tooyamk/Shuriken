#include "ShaderParameterFactory.h"
#include "modules/graphics/ShaderParameter.h"

namespace aurora::modules::graphics {
	ShaderParameterFactory::~ShaderParameterFactory() {
		clear();
	}

	ShaderParameter* ShaderParameterFactory::add(const std::string& name, ShaderParameter* parameter) {
		if (auto itr = _parameters.find(name); parameter) {
			if (itr == _parameters.end()) {
				parameter->ref();
				_parameters.emplace(name, parameter);
			} else if (itr->second != parameter) {
				parameter->ref();
				itr->second->unref();
				itr->second = parameter;
			}
		} else if (itr != _parameters.end()) {
			itr->second->unref();
			_parameters.erase(itr);
		}

		return parameter;
	}

	void ShaderParameterFactory::remove(const std::string& name) {
		if (auto itr = _parameters.find(name); itr != _parameters.end()) {
			itr->second->unref();
			_parameters.erase(itr);
		}
	}

	void ShaderParameterFactory::clear() {
		for (auto& itr : _parameters) itr.second->unref();
		_parameters.clear();
	}
}