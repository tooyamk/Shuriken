#pragma once

#include <dxc/Support/Global.h>
#include <dxc/Support/Unicode.h>
//#include <dxc/Support/WinAdapter.h>
#include <dxc/Support/WinIncludes.h>

#include "dxc/dxcapi.h"

namespace aurora::shader_translator {
	enum class ShaderLanguage : unsigned char {
		UNKNOWN,
		HLSL,
		DXIL,
		SPIRV,
		GLSL,
		GSSL,
		MSL
	};


	class ShaderSource {
	public:
		ShaderSource();

		ShaderLanguage language;
		char* data;
		unsigned int dataSize;
	};


	class ShaderTranslator {
	public:
		ShaderTranslator();
		
		void translate(const ShaderSource& source);

	private:
		HMODULE _dxcll;
		DxcCreateInstanceProc _dxCreateInsFn;

		CComPtr<IDxcLibrary> _lib;
		CComPtr<IDxcCompiler> _compiler;
	};
}