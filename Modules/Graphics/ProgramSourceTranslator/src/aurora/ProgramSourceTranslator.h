#pragma once

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/dxcapi.h"

#include "aurora/modules/graphics/IProgramSourceTranslator.h"
#include "aurora/Debug.h"
#include "aurora/DynamicLib.h"

namespace aurora::modules::graphics::program_source_translator {
	class ProgramSourceTranslator : public IProgramSourceTranslator {
	public:
		ProgramSourceTranslator(Ref* loader, const std::string_view& dxc);
		virtual ~ProgramSourceTranslator();

		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) override;

	protected:
		virtual ScopeGuard AE_CALL _destruction() const override {
			auto l = _loader;
			return [l]() {};
		}

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


		RefPtr<Ref> _loader;
		DynamicLib _dxcDll;

		CComPtr<IDxcLibrary> _dxcLib;
		CComPtr<IDxcCompiler> _dxcompiler;

		void _spirvTo(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst);
	};
}

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		if (!args) {
			printdln("Module create err, no args");
			return nullptr;
		}

		std::string_view dxc;
		if (auto so = args->tryGetPtr("dxc"); so) dxc = so->toStringView();
		if (dxc.empty()) {
			printdln("Module create err, no dxc");
			return nullptr;
		}

		return new program_source_translator::ProgramSourceTranslator(loader, dxc);
	}
}
#endif