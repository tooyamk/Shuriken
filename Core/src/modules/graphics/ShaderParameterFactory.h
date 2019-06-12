#pragma once

#include "modules/graphics/ShaderParameter.h"
#include <unordered_map>

namespace aurora::modules::graphics {
	enum class ShaderParameterType : ui8;
	class ShaderParameter;


	class AE_DLL ShaderParameterFactory : public Ref {
	public:
		~ShaderParameterFactory();

		inline ShaderParameter* AE_CALL get(const std::string& name) const {
			auto itr = _parameters.find(name);
			return itr == _parameters.end() ? nullptr : itr->second;
		}
		inline ShaderParameter* AE_CALL get(const std::string& name, ShaderParameterType type) const {
			auto itr = _parameters.find(name);
			return itr == _parameters.end() ? nullptr : (itr->second->getType() == type ? itr->second : nullptr);
		}

		ShaderParameter* AE_CALL add(const std::string& name, ShaderParameter* parameter);
		void AE_CALL remove(const std::string& name);
		void AE_CALL clear();

	private:
		std::unordered_map<std::string, ShaderParameter*> _parameters;
	};
}