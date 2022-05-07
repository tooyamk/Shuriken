#include "ShaderTranspiler.h"
//#include "spirv-tools/libspirv.h"
//#include "spirv.hpp"
//#include "spirv_cross.hpp"
#include "spirv_cross/spirv_msl.hpp"
#include "srk/ShaderDefine.h"
#include "srk/String.h"
#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk::modules::graphics::shader_transpiler {
	ShaderTranspiler::MyIncludeHandler::MyIncludeHandler(IDxcLibrary* lib, const IShaderTranspiler::IncludeHandler& handler) :
		_lib(lib),
		_handler(handler),
		_ref(0) {
	}

	HRESULT ShaderTranspiler::MyIncludeHandler::LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) {
		if ((pFilename[0] == L'.') && (pFilename[1] == L'/')) pFilename += 2;

		if (_handler) {
			 ByteArray data = _handler(String::UnicodeToUtf8<std::wstring_view, std::string>(std::wstring_view(pFilename)));
			 return _lib->CreateBlobWithEncodingOnHeapCopy(data.getSource(), data.getLength(), CP_UTF8, reinterpret_cast<IDxcBlobEncoding**>(ppIncludeSource));
		} else {
			*ppIncludeSource = nullptr;
		}

		return E_FAIL;
	}

	ULONG ShaderTranspiler::MyIncludeHandler::AddRef() {
		++_ref;
		return _ref;
	}

	ULONG ShaderTranspiler::MyIncludeHandler::Release() {
		--_ref;
		ULONG result = _ref;
		if (result == 0) delete this;
		return result;
	}

	HRESULT ShaderTranspiler::MyIncludeHandler::QueryInterface(REFIID riid, void** ppvObject) {
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


	ShaderTranspiler::ShaderTranspiler() :
		_loader(),
		_dxcLib(nullptr),
		_dxcompiler(nullptr) {
		
	}

	ShaderTranspiler::~ShaderTranspiler() {
	}

	bool ShaderTranspiler::init(Ref* loader, const std::string_view& dxc) {
		using namespace std::literals;

		if (_dxcDll.load(dxc)) {
			if (auto fn = (DxcCreateInstanceProc)_dxcDll.getSymbolAddress("DxcCreateInstance"sv); fn) {
				if (auto hr = fn(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&_dxcLib); hr < 0) return false;
				if (auto hr = fn(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&_dxcompiler); hr < 0) return false;

				_loader = loader;
				return true;
			}
		}

		return false;
	}

	ProgramSource ShaderTranspiler::translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) {
		using namespace std::literals;

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
			if (targetLanguage != ProgramLanguage::DXIL) dxcArgStrings.emplace_back(L"-spirv");

			std::vector<const wchar_t*> dxcArgs;
			dxcArgs.reserve(dxcArgStrings.size());
			for (const auto& arg : dxcArgStrings) dxcArgs.emplace_back(arg.data());
			
			CComPtr<IDxcIncludeHandler> includeHandler = new MyIncludeHandler(_dxcLib, handler);
			CComPtr<IDxcOperationResult> compileResult;
			IFT(_dxcompiler->Compile(sourceBlob, L"", String::Utf8ToUnicode(ProgramSource::getEntryPoint(source)).data(), profile.data(),
				dxcArgs.data(), (UINT32)(dxcArgs.size()), dxcDefines.data(),
				(UINT32)(dxcDefines.size()), includeHandler, &compileResult));

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

	void ShaderTranspiler::_spirvTo(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
		ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst) {
		using namespace std::literals;

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

	void ShaderTranspiler::_spirvToGLSL(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
		ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst, spv::ExecutionModel model) {
		using namespace std::literals;

		spirv_cross::CompilerGLSL compiler((uint32_t*)sourceData, sourceDataSize / sizeof(uint32_t));

		compiler.set_entry_point(ProgramSource::getEntryPoint(source), model);
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
				compiler.set_name(remap.combined_id, COMBINED_TEXTURE_SAMPLER + String::toString(texName.size()) + "s" + texName + "s" + samplerName);
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
			printaln(L"ProgramSourceTranslator::translate spirv to glsl/gssl error : "sv, error.what());
		}
	}

	void ShaderTranspiler::_spirvToMSL(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
		ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst, spv::ExecutionModel model) {
		using namespace std::literals;

		spirv_cross::CompilerMSL compiler((uint32_t*)sourceData, sourceDataSize / sizeof(uint32_t));

		compiler.set_entry_point(ProgramSource::getEntryPoint(source), model);

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
			printaln(L"ProgramSourceTranslator::translate spirv to msl error : "sv, error.what());
		}
	}
}