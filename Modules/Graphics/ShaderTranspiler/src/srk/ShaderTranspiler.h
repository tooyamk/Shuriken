#pragma once

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/dxcapi.h"

#include "spirv_cross/spirv.hpp"

#include "srk/modules/graphics/IShaderTranspiler.h"
#include "srk/Debug.h"
#include "srk/DynamicLibraryLoader.h"

namespace srk::modules::graphics::shader_transpiler {
	class ShaderTranspiler : public IShaderTranspiler {
	public:
		ShaderTranspiler();
		virtual ~ShaderTranspiler();

		void operator delete(ShaderTranspiler* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~ShaderTranspiler();
			::operator delete(p);
		}

		bool SRK_CALL init(Ref* loader, const std::string_view& dxc);
		virtual ProgramSource SRK_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) override;

	private:
		class MyIncludeHandler : public IDxcIncludeHandler {
		public:
			MyIncludeHandler(IDxcLibrary* lib, const IShaderTranspiler::IncludeHandler& handler);

			HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
			ULONG STDMETHODCALLTYPE AddRef() override;
			ULONG STDMETHODCALLTYPE Release() override;
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

		private:
			IDxcLibrary* _lib;
			IShaderTranspiler::IncludeHandler _handler;
			std::atomic<ULONG> _ref;
		};


		IntrusivePtr<Ref> _loader;
		DynamicLibraryLoader _dxcDll;

		CComPtr<IDxcLibrary> _dxcLib;
		CComPtr<IDxcCompiler> _dxcompiler;

		void SRK_CALL _spirvTo(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst);
		void SRK_CALL _spirvToGLSL(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst, spv::ExecutionModel model);
		void SRK_CALL _spirvToMSL(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst, spv::ExecutionModel model);
	};
}

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::graphics {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		if (!args) {
			printaln(L"Module create err, no args"sv);
			return nullptr;
		}

		std::string_view dxc;
		if (auto so = args->tryGetPtr("dxc"); so) dxc = so->toStringView();
		if (dxc.empty()) {
			printaln(L"Module create err, no dxc"sv);
			return nullptr;
		}

		auto transpiler = new shader_transpiler::ShaderTranspiler();
		if (!transpiler->init(loader, dxc)) {
			delete transpiler;
			transpiler = nullptr;
		}

		return transpiler;
	}
}
#endif