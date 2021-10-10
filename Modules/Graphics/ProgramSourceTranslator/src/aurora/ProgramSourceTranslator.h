#pragma once

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/dxcapi.h"

#include "spirv_cross/spirv.hpp"

#include "aurora/modules/graphics/IProgramSourceTranslator.h"
#include "aurora/Debug.h"
#include "aurora/DynamicLib.h"

namespace aurora::modules::graphics::program_source_translator {
	class ProgramSourceTranslator : public IProgramSourceTranslator {
	public:
		ProgramSourceTranslator();
		virtual ~ProgramSourceTranslator();

		void operator delete(ProgramSourceTranslator* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~ProgramSourceTranslator();
			::operator delete(p);
		}

		bool AE_CALL init(Ref* loader, const std::string_view& dxc);
		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) override;

	private:
		class MyIncludeHandler : public IDxcIncludeHandler {
		public:
			MyIncludeHandler(IDxcLibrary* lib, const IProgramSourceTranslator::IncludeHandler& handler);

			HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
			ULONG STDMETHODCALLTYPE AddRef() override;
			ULONG STDMETHODCALLTYPE Release() override;
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

		private:
			IDxcLibrary* _lib;
			IProgramSourceTranslator::IncludeHandler _handler;
			std::atomic<ULONG> _ref;
		};


		IntrusivePtr<Ref> _loader;
		DynamicLib _dxcDll;

		CComPtr<IDxcLibrary> _dxcLib;
		CComPtr<IDxcCompiler> _dxcompiler;

		void AE_CALL _spirvTo(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst);
		void AE_CALL _spirvToGLSL(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst, spv::ExecutionModel model);
		void AE_CALL _spirvToMSL(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst, spv::ExecutionModel model);
	};
}

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		if (!args) {
			printdln(L"Module create err, no args"sv);
			return nullptr;
		}

		std::string_view dxc;
		if (auto so = args->tryGetPtr("dxc"); so) dxc = so->toStringView();
		if (dxc.empty()) {
			printdln(L"Module create err, no dxc"sv);
			return nullptr;
		}

		auto translator = new program_source_translator::ProgramSourceTranslator();
		if (!translator->init(loader, dxc)) {
			delete translator;
			translator = nullptr;
		}

		return translator;
	}
}
#endif