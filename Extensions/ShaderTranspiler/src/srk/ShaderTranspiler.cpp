#include "ShaderTranspilerImpl.h"

namespace srk::extensions {
	ShaderTranspiler::ShaderTranspiler(shader_transpiler::Impl* impl) :
		_impl(impl) {
	}

	ShaderTranspiler::~ShaderTranspiler() {
		delete _impl;
	}

	modules::graphics::ProgramSource ShaderTranspiler::translate(const modules::graphics::ProgramSource& source, const Options& options, modules::graphics::ProgramLanguage targetLanguage, const std::string_view& targetVersion, const modules::graphics::ProgramDefine* defines, size_t numDefines, const modules::graphics::ProgramIncludeHandler& handler) {
		return _impl->translate(source, options, targetLanguage, targetVersion, defines, numDefines, handler);
	}

	IntrusivePtr<ShaderTranspiler> ShaderTranspiler::create() {
		using namespace std::string_view_literals;

#if SRK_OS == SRK_OS_WINDOWS
#	ifdef SRK_DEBUG
		return create("dxcompilerd"sv);
#	else
		return create("dxcompiler"sv);
#	endif
#else
#	ifdef SRK_DEBUG
		return create("libdxcompilerd"sv);
#	else
		return create("libdxcompiler"sv);
#	endif
#endif
	}

	IntrusivePtr<ShaderTranspiler> ShaderTranspiler::create(const std::string_view& dxcompiler) {
		using namespace std::string_view_literals;

		DynamicLibraryLoader dxcLoader;
		if (!dxcLoader.load(dxcompiler)) return nullptr;

		if (auto fn = (DxcCreateInstanceProc)dxcLoader.getSymbolAddress("DxcCreateInstance"sv); fn) {
			CComPtr<IDxcLibrary> dxcLib;
			CComPtr<IDxcCompiler> dxcInstance;

			if (auto hr = fn(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&dxcLib); hr < 0) return nullptr;
			if (auto hr = fn(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&dxcInstance); hr < 0) return nullptr;
			
			return new ShaderTranspiler(new shader_transpiler::Impl(std::move(dxcLoader), dxcLib, dxcInstance));
		}

		return nullptr;
	}
}