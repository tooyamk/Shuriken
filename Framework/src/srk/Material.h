#pragma once

#include "srk/Shader.h"
#include "srk/ShaderParameter.h"

namespace srk {
	class SRK_FW_DLL Material : public Ref {
	public:
		Material();

		inline const IntrusivePtr<Shader> SRK_CALL getShader() const {
			return _shader;
		}

		inline IntrusivePtr<Shader> SRK_CALL getShader() {
			return _shader;
		}

		inline void SRK_CALL setShader(Shader* shader) {
			_shader = shader;
		}

		inline ShaderDefineCollection* SRK_CALL getDefines() const {
			return _defines;
		}
		inline void SRK_CALL setDefines(ShaderDefineCollection* defines) {
			_defines = defines;
		}

		inline ShaderParameterCollection* SRK_CALL getParameters() const {
			return _parameters;
		}
		inline void SRK_CALL setParameters(ShaderParameterCollection* parameters) {
			_parameters = parameters;
		}

	protected:
		IntrusivePtr<Shader> _shader;
		IntrusivePtr<ShaderDefineCollection> _defines;
		IntrusivePtr<ShaderParameterCollection> _parameters;
	};
}