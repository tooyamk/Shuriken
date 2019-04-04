#pragma once

#include <windows.h>

#include <dxc/Support/Global.h>
#include <dxc/Support/Unicode.h>
//#include <dxc/Support/WinAdapter.h>
#include <dxc/Support/WinIncludes.h>

#include "dxc/dxcapi.h"

namespace shader_translator {
	class ShaderTranslator {
	public:
		ShaderTranslator();
		
		void translate(char* src);

	private:
		HMODULE _dxcll;
		DxcCreateInstanceProc _dxCreateInsFn;

		CComPtr<IDxcLibrary> _lib;
		CComPtr<IDxcCompiler> _compiler;
	};
}