#pragma once

#include "srk/Global.h"

namespace srk {
	class SRK_FW_DLL ShaderPredefine {
	public:
		inline static const std::string BINORMAL0 = { "BINORMAL0" };
		inline static const std::string COLOR0 = { "COLOR0" };
		inline static const std::string NORMAL0 = { "NORMAL0" };
		inline static const std::string POSITION0 = { "POSITION0" };
		inline static const std::string TANGENT0 = { "TANGENT0" };
		inline static const std::string UV0 = { "UV0" };

		inline static const std::string MATRIX_LW = { "_matrix_lw" };
		inline static const std::string MATRIX_LV = { "_matrix_lv" };
		inline static const std::string MATRIX_LP = { "_matrix_lp" };
		inline static const std::string MATRIX_WV = { "_matrix_wv" };
		inline static const std::string MATRIX_WP = { "_matrix_wp" };

		inline static const std::string CAMERA_POS = { "_cameraPos" };

		inline static const std::string AMBIENT_COLOR = { "_ambientColor" };
		inline static const std::string DIFFUSE_COLOR = { "_diffuseColor" };
		inline static const std::string SPECULAR_COLOR = { "_specularColor" };

		inline static const std::string DIFFUSE_TEXTURE = { "_diffuseTex" };
	};
}