#pragma once

#include "aurora/Shader.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"

namespace aurora {
	class AE_DLL Material : public Ref {
	public:
		Material();

		inline Shader* AE_CALL getShader() const {
			return _shader;
		}
		inline void AE_CALL setShader(Shader* shader) {
			_shader = shader;
		}

		inline ShaderDefineCollection& AE_CALL getDefines() {
			return _defines;
		}
		inline const ShaderDefineCollection& AE_CALL getDefines() const {
			return _defines;
		}

		inline ShaderParameterCollection& AE_CALL getParameters() {
			return _parameters;
		}
		inline const ShaderParameterCollection& AE_CALL getParameters() const {
			return _parameters;
		}

	protected:
		RefPtr<Shader> _shader;
		ShaderDefineCollection _defines;
		ShaderParameterCollection _parameters;
	};
}