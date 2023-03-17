#pragma once

#include "ShaderTranspiler.h"
#include "srk/Printer.h"
#include "srk/DynamicLibraryLoader.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/dxcapi.h"

#include "spirv_cross/spirv.hpp"
#include "spirv_cross/spirv_msl.hpp"

namespace srk::extensions::shader_transpiler {
	class Impl {
	public:
		Impl(DynamicLibraryLoader* dxcLloader, const CComPtr<IDxcLibrary>& dxcLib, const CComPtr<IDxcCompiler>& dxcInstance) :
			_dxcLoader(dxcLloader),
			_dxcLib(dxcLib),
			_dxcInstance(dxcInstance) {

		}

		~Impl() {
			_dxcInstance = nullptr;
			_dxcLib = nullptr;

			delete _dxcLoader;
		}

		modules::graphics::ProgramSource SRK_CALL translate(const modules::graphics::ProgramSource& source, const ShaderTranspiler::Options& options, modules::graphics::ProgramLanguage targetLanguage, const std::string_view& targetVersion, const modules::graphics::ProgramDefine* defines, size_t numDefines, const modules::graphics::ProgramIncludeHandler& handler) {
			using namespace std::literals;
			using namespace srk::modules::graphics;

			ProgramSource dst;

			if (!source.isValid() || targetLanguage == ProgramLanguage::UNKNOWN) return std::move(dst);

			if (source.language == targetLanguage) {
				dst.language = targetLanguage;
				dst.stage = source.stage;
				dst.version = targetVersion.empty() ? source.version : targetVersion;
				dst.data.setCapacity(source.data.getLength());
				dst.data.write<ba_vt::BYTE>(source.data.getSource(), source.data.getLength());
				return std::move(dst);
			}

			if (source.language == ProgramLanguage::HLSL) {
				auto profile = String::Utf8ToUnicode(ProgramSource::toHLSLShaderModel(source.stage, source.version));

				std::vector<std::wstring> defineStrs(numDefines << 1);
				std::vector<DxcDefine> dxcDefines(numDefines);
				for (decltype(numDefines) i = 0; i < numDefines; ++i) {
					auto& dxcDef = dxcDefines[i];

					auto j = i << 1;
					auto def = defines + j;
					defineStrs[j] = String::Utf8ToUnicode(def->name);
					dxcDef.Name = defineStrs[j].data();

					++j;
					defineStrs[j] = String::Utf8ToUnicode(def->value);
					dxcDef.Value = defineStrs[j].data();
				}

				CComPtr<IDxcBlobEncoding> sourceBlob;
				if (auto hr = _dxcLib->CreateBlobWithEncodingOnHeapCopy(source.data.getSource(), source.data.getLength(), CP_UTF8, &sourceBlob); hr < 0) {
					printaln(L"ProgramSourceTranslator::translate dxc failed"sv);
					return std::move(dst);
				}
				IFTARG(sourceBlob->GetBufferSize() >= 4);

				//std::wstring shaderNameUtf16;
				//Unicode::UTF8ToUTF16String("fileName", &shaderNameUtf16);

				//std::wstring entryPointUtf16;
				//Unicode::UTF8ToUTF16String("main", &entryPointUtf16);

				std::vector<std::wstring> dxcArgStrings;
				dxcArgStrings.emplace_back(L"-O3");
				dxcArgStrings.emplace_back(L"-fvk-use-gl-layout");
				//dxcArgStrings.emplace_back(L"-auto-binding-space 0")
				//dxcArgStrings.emplace_back(L"-auto-binding-space " + String::Utf8ToUnicode(String::toString((std::underlying_type_t<ProgramStage>)source.stage)));
				//dxcArgStrings.emplace_back(L"-fspv-target-env=vulkan1.0");
				//dxcArgStrings.emplace_back(L"-fspv-reflect");
				if (targetLanguage != ProgramLanguage::DXIL) {
					dxcArgStrings.emplace_back(L"-spirv");
					if (options.spirv.descriptorSet0BindingOffset) {
						auto offset = String::Utf8ToUnicode(String::toString(options.spirv.descriptorSet0BindingOffset));
						//dxcArgStrings.emplace_back(L"-fvk-bind-globals");
						//dxcArgStrings.emplace_back(offset);
						//dxcArgStrings.emplace_back(L"0");
						dxcArgStrings.emplace_back(L"-fvk-b-shift");
						dxcArgStrings.emplace_back(offset);
						dxcArgStrings.emplace_back(L"0");
						dxcArgStrings.emplace_back(L"-fvk-t-shift");
						dxcArgStrings.emplace_back(offset);
						dxcArgStrings.emplace_back(L"0");
						dxcArgStrings.emplace_back(L"-fvk-s-shift");
						dxcArgStrings.emplace_back(offset);
						dxcArgStrings.emplace_back(L"0");
						dxcArgStrings.emplace_back(L"-fvk-u-shift");
						dxcArgStrings.emplace_back(offset);
						dxcArgStrings.emplace_back(L"0");
						dxcArgStrings.emplace_back(L"-fvk-auto-shift-bindings");
					}
				}

				std::vector<const wchar_t*> dxcArgs;
				dxcArgs.reserve(dxcArgStrings.size());
				for (const auto& arg : dxcArgStrings) dxcArgs.emplace_back(arg.data());

				CComPtr<IDxcIncludeHandler> includeHandler = new MyIncludeHandler(_dxcLib, handler);
				CComPtr<IDxcOperationResult> compileResult;
				IFT(_dxcInstance->Compile(sourceBlob, L"", String::Utf8ToUnicode(source.getEntryPoint()).data(), profile.data(),
					dxcArgs.data(), dxcArgs.size(), dxcDefines.data(), dxcDefines.size(), includeHandler, &compileResult));

				HRESULT status;
				IFT(compileResult->GetStatus(&status));

				CComPtr<IDxcBlobEncoding> errors;
				IFT(compileResult->GetErrorBuffer(&errors));
				if (errors) {
					if (errors->GetBufferSize() > 0) {
						printaln(L"ProgramSourceTranslator::translate error or warning : "sv, std::string_view((char*)errors->GetBufferPointer(), errors->GetBufferSize()));
						//ret.errorWarningMsg = CreateBlob(errors->GetBufferPointer(), static_cast<uint32_t>(errors->GetBufferSize()));
					}
					errors = nullptr;
				}

				if (SUCCEEDED(status)) {
					CComPtr<IDxcBlob> program;
					IFT(compileResult->GetResult(&program));
					compileResult = nullptr;
					if (program != nullptr) _spirvTo(source, (uint8_t*)program->GetBufferPointer(), (uint32_t)program->GetBufferSize(), targetLanguage, targetVersion, dst);
				}
			} else if (source.language == ProgramLanguage::SPIRV) {
				_spirvTo(source, source.data.getSource(), source.data.getLength(), targetLanguage, targetVersion, dst);
			}

			return std::move(dst);
		}

	private:
		DynamicLibraryLoader* _dxcLoader;

		CComPtr<IDxcLibrary> _dxcLib;
		CComPtr<IDxcCompiler> _dxcInstance;

		class MyIncludeHandler : public IDxcIncludeHandler {
		public:
			MyIncludeHandler(IDxcLibrary* lib, const modules::graphics::ProgramIncludeHandler& handler) :
				_lib(lib),
				_handler(handler),
				_ref(0) {
			}

			HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override {
				using namespace srk::modules::graphics;

				if ((pFilename[0] == L'.') && (pFilename[1] == L'/')) pFilename += 2;

				if (_handler) {
					ProgramIncludeInfo pii;
					pii.file = String::UnicodeToUtf8<std::wstring_view, std::string>(std::wstring_view(pFilename));
					ByteArray data = _handler(pii);
					return _lib->CreateBlobWithEncodingOnHeapCopy(data.getSource(), data.getLength(), CP_UTF8, reinterpret_cast<IDxcBlobEncoding**>(ppIncludeSource));
				} else {
					*ppIncludeSource = nullptr;
				}

				return E_FAIL;
			}

			ULONG STDMETHODCALLTYPE AddRef() override {
				++_ref;
				return _ref;
			}

			ULONG STDMETHODCALLTYPE Release() override {
				--_ref;
				ULONG result = _ref;
				if (result == 0) delete this;
				return result;
			}

			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
				if (IsEqualIID(riid, __uuidof(IDxcIncludeHandler))) {
					*ppvObject = dynamic_cast<IDxcIncludeHandler*>(this);
					this->AddRef();
					return S_OK;
				} else if (IsEqualIID(riid, __uuidof(IUnknown))) {
					*ppvObject = dynamic_cast<IUnknown*>(this);
					this->AddRef();
					return S_OK;
				} else {
					return E_NOINTERFACE;
				}
			}

		private:
			IDxcLibrary* _lib;
			modules::graphics::ProgramIncludeHandler _handler;
			std::atomic<ULONG> _ref;
		};


		void SRK_CALL _spirvTo(const modules::graphics::ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize, modules::graphics::ProgramLanguage targetLanguage, const std::string_view& targetVersion, modules::graphics::ProgramSource& dst) {
			using namespace std::literals;
			using namespace srk::modules::graphics;

			spv::ExecutionModel model;
			switch (source.stage) {
			case ProgramStage::VS:
				model = spv::ExecutionModelVertex;
				break;
			case ProgramStage::PS:
				model = spv::ExecutionModelFragment;
				break;
			case ProgramStage::GS:
				model = spv::ExecutionModelGeometry;
				break;
			case ProgramStage::CS:
				model = spv::ExecutionModelGLCompute;
				break;
			case ProgramStage::HS:
				model = spv::ExecutionModelTessellationControl;
				break;
			case ProgramStage::DS:
				model = spv::ExecutionModelTessellationEvaluation;
				break;
			default:
				return;
			}

			switch (targetLanguage) {
			case ProgramLanguage::DXIL:
			case ProgramLanguage::SPIRV:
			{
				dst.language = targetLanguage;
				dst.stage = source.stage;
				if (targetLanguage == ProgramLanguage::DXIL) dst.version = targetVersion.empty() ? source.version : targetVersion;
				dst.data.setCapacity(sourceDataSize);
				dst.data.write<ba_vt::BYTE>(sourceData, sourceDataSize);

				break;
			}
			case ProgramLanguage::GLSL:
			case ProgramLanguage::GSSL:
				_spirvToGLSL(source, sourceData, sourceDataSize, targetLanguage, targetVersion, dst, model);
				break;
			case ProgramLanguage::MSL:
				_spirvToMSL(source, sourceData, sourceDataSize, targetLanguage, targetVersion, dst, model);
				break;
			default:
				break;
			}
		}

		void SRK_CALL _spirvToGLSL(const modules::graphics::ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize, modules::graphics::ProgramLanguage targetLanguage, const std::string_view& targetVersion, modules::graphics::ProgramSource& dst, spv::ExecutionModel model) {
			using namespace std::literals;
			using namespace srk::modules::graphics;

			spirv_cross::CompilerGLSL compiler((uint32_t*)sourceData, sourceDataSize / sizeof(uint32_t));

			compiler.set_entry_point(source.getEntryPoint(), model);
			//compiler.require_extension("GL_ARB_fragment_coord_conventions");
			//compiler.set_execution_mode(spv::ExecutionModeOriginUpperLeft);
			auto opts = compiler.get_common_options();
			if (!targetVersion.empty()) opts.version = String::toNumber<decltype(opts.version)>(targetVersion);
			opts.es = targetLanguage == ProgramLanguage::GSSL;
			opts.force_temporary = false;
			opts.separate_shader_objects = true;
			opts.flatten_multidimensional_arrays = false;
			opts.enable_420pack_extension = targetLanguage == ProgramLanguage::GLSL && (targetVersion.empty() || opts.version >= 420);
			opts.vulkan_semantics = false;
			opts.vertex.fixup_clipspace = false;
			opts.vertex.flip_vert_y = false;
			opts.vertex.support_nonzero_base_instance = true;
			//opts.emit_uniform_buffer_as_plain_uniforms = true;
			compiler.set_common_options(opts);

			auto resources = compiler.get_shader_resources();

			for (auto& val : resources.uniform_buffers) {
				compiler.unset_decoration(val.id, spv::DecorationBinding);
				if (val.name == "type.$Globals"sv) {
					compiler.set_name(val.base_type_id, val.name + String::toString((std::underlying_type_t<ProgramStage>)source.stage));
				}
			}
			/*
			for (auto& val : resources.uniform_buffers) {
				auto& type = compiler.get_type(val.base_type_id);
				auto numMembers = type.member_types.size();
				for (size_t i = 0; i < numMembers; ++i) {
					//auto& name = compiler.get_member_name(type.self, i);
					//auto& memberType = compiler.get_type(type.member_types[i]);
					if (compiler.has_member_decoration(type.self, i, spv::DecorationRowMajor)) {
						compiler.unset_member_decoration(type.self, i, spv::DecorationRowMajor);
						compiler.set_member_decoration(type.self, i, spv::DecorationColMajor);
					}
				}
			}
			*/
			for (auto& val : resources.stage_inputs) {
				auto& name = compiler.get_name(val.id);
				if (name.size() > 7 && std::string_view(name.data(), 7) == "in.var."sv) {
					compiler.set_name(val.id, std::string(name.data() + 7));
				}
			}

			if (auto sampler = compiler.build_dummy_sampler_for_combined_images(); (uint32_t)sampler != 0) {
				compiler.set_decoration(sampler, spv::DecorationDescriptorSet, 0);
				compiler.set_decoration(sampler, spv::DecorationBinding, 0);
			}

			//auto sr = compiler.get_shader_resources();
			//for (auto& in : sr.stage_inputs) compiler.set_name(in.id, in.name.substr(7));
			//for (auto& in : sr.stage_outputs) compiler.set_name(in.id, in.name.substr(8));

			compiler.build_combined_image_samplers();
			for (auto& remap : compiler.get_combined_image_samplers()) {
				auto& texName = compiler.get_name(remap.image_id);
				auto& samplerName = compiler.get_name(remap.sampler_id);

				if (samplerName.rfind("DummySampler") == std::string::npos) {
					compiler.set_name(remap.combined_id, COMBINED_TEXTURE_SAMPLER_HEADER + String::toString(texName.size()) + "s" + texName + "s" + samplerName);
				} else {
					compiler.set_name(remap.combined_id, texName);
				}
			}

			try {
				auto str = compiler.compile();

				dst.language = targetLanguage;
				dst.stage = source.stage;
				dst.version = String::toString(opts.version);
				dst.data.setCapacity(str.size());
				dst.data.write<ba_vt::BYTE>((uint8_t*)str.data(), str.size());
			} catch (spirv_cross::CompilerError& error) {
				printaln(L"ProgramSourceTranslator::translate spirv to glsl/gssl error : "sv, std::string_view(error.what()));
			}
		}

		void SRK_CALL _spirvToMSL(const modules::graphics::ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize, modules::graphics::ProgramLanguage targetLanguage, const std::string_view& targetVersion, modules::graphics::ProgramSource& dst, spv::ExecutionModel model) {
			using namespace std::literals;
			using namespace srk::modules::graphics;

			spirv_cross::CompilerMSL compiler((uint32_t*)sourceData, sourceDataSize / sizeof(uint32_t));

			compiler.set_entry_point(source.getEntryPoint(), model);

			auto opts = compiler.get_common_options();
			if (!targetVersion.empty()) opts.version = String::toNumber<decltype(opts.version)>(targetVersion);
			opts.es = false;
			opts.force_temporary = false;
			opts.separate_shader_objects = true;
			opts.flatten_multidimensional_arrays = false;
			opts.enable_420pack_extension = false;
			opts.vulkan_semantics = false;
			opts.vertex.fixup_clipspace = false;
			opts.vertex.flip_vert_y = false;
			opts.vertex.support_nonzero_base_instance = true;
			compiler.set_common_options(opts);

			auto mslOpts = compiler.get_msl_options();
			if (!targetVersion.empty()) mslOpts.msl_version = String::toNumber<decltype(mslOpts.msl_version)>(targetVersion);
			mslOpts.swizzle_texture_samples = false;
			compiler.set_msl_options(mslOpts);

			try {
				auto str = compiler.compile();

				dst.language = targetLanguage;
				dst.stage = source.stage;
				dst.version = String::toString(mslOpts.msl_version);
				dst.data.setCapacity(str.size());
				dst.data.write<ba_vt::BYTE>((uint8_t*)str.data(), str.size());
			} catch (spirv_cross::CompilerError& error) {
				printaln(L"ProgramSourceTranslator::translate spirv to msl error : "sv, std::string_view(error.what()));
			}
		}
	};
}