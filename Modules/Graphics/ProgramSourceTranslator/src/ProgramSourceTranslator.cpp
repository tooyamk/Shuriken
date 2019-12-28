#include "ProgramSourceTranslator.h"
//#include "spirv-tools/libspirv.h"
//#include "spirv.hpp"
//#include "spirv_cross.hpp"
#include "spirv_msl.hpp"
#include "base/String.h"
#include "modules/graphics/IGraphicsModule.h"

namespace aurora::modules::graphics::program_source_translator {
	ProgramSourceTranslator::ProgramSourceTranslator(Ref* loader, const char* dxc) :
		_loader(loader),
		_dxcLib(nullptr),
		_dxcompiler(nullptr) {
		if (_dxcDll.load(dxc)) {
			if (auto fn = (DxcCreateInstanceProc)_dxcDll.getSymbolAddress("DxcCreateInstance"); fn) {
				IFT(fn(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&_dxcLib));
				IFT(fn(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&_dxcompiler));
			}
		}
	}

	ProgramSourceTranslator::~ProgramSourceTranslator() {
	}

	ProgramSource ProgramSourceTranslator::translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string& targetVersion) {
		ProgramSource dst;

		if (!source.isValid() || targetLanguage == ProgramLanguage::UNKNOWN) return std::move(dst);

		if (source.language == targetLanguage) {
			dst.language = targetLanguage;
			dst.stage = source.stage;
			dst.version = targetVersion.empty() ? source.version : targetVersion;
			dst.data.setCapacity(source.data.getLength());
			dst.data.writeBytes(source.data.getSource(), source.data.getLength());
			return std::move(dst);
		}

		if (source.language == ProgramLanguage::HLSL) {
			auto profile = String::Utf8ToUnicode(ProgramSource::toHLSLShaderModel(source.stage, source.version));

			std::vector<DxcDefine> dxcDefines;

			CComPtr<IDxcBlobEncoding> sourceBlob;
			IFT(_dxcLib->CreateBlobWithEncodingOnHeapCopy(source.data.getSource(), source.data.getLength(), CP_UTF8, &sourceBlob));
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
			for (const auto& arg : dxcArgStrings) dxcArgs.emplace_back(arg.c_str());

			//CComPtr<IDxcIncludeHandler> includeHandler = new ScIncludeHandler(std::move(source.loadIncludeCallback));
			CComPtr<IDxcOperationResult> compileResult;
			IFT(_dxcompiler->Compile(sourceBlob, L"", String::Utf8ToUnicode(ProgramSource::getEntryPoint(source)).c_str(), profile.c_str(),
				dxcArgs.data(), (UINT32)(dxcArgs.size()), dxcDefines.data(),
				(UINT32)(dxcDefines.size()), nullptr, &compileResult));

			HRESULT status;
			IFT(compileResult->GetStatus(&status));

			CComPtr<IDxcBlobEncoding> errors;
			IFT(compileResult->GetErrorBuffer(&errors));
			if (errors != nullptr) {
				if (errors->GetBufferSize() > 0) {
					println("hlsl to spirv error or warning : ", std::string((char*)errors->GetBufferPointer(), errors->GetBufferSize()));
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

	void ProgramSourceTranslator::_spirvTo(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize, 
		ProgramLanguage targetLanguage, const std::string& targetVersion, ProgramSource& dst) {

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
			dst.data.writeBytes(sourceData, sourceDataSize);

			break;
		}
		case ProgramLanguage::GLSL:
		case ProgramLanguage::GSSL:
		{
			spirv_cross::CompilerGLSL compiler((uint32_t*)sourceData, sourceDataSize / sizeof(uint32_t));

			compiler.set_entry_point(ProgramSource::getEntryPoint(source), model);

			auto opts = compiler.get_common_options();
			if (!targetVersion.empty()) opts.version = std::stoi(targetVersion);
			opts.es = targetLanguage == ProgramLanguage::GSSL;
			opts.force_temporary = false;
			opts.separate_shader_objects = true;
			opts.flatten_multidimensional_arrays = false;
			opts.enable_420pack_extension = targetLanguage == ProgramLanguage::GLSL && (targetVersion.empty() || opts.version >= 420);
			opts.vulkan_semantics = false;
			opts.vertex.fixup_clipspace = false;
			opts.vertex.flip_vert_y = false;
			opts.vertex.support_nonzero_base_instance = true;
			compiler.set_common_options(opts);

			if (auto sampler = compiler.build_dummy_sampler_for_combined_images(); sampler != 0) {
				compiler.set_decoration(sampler, spv::DecorationDescriptorSet, 0);
				compiler.set_decoration(sampler, spv::DecorationBinding, 0);
			}

			//auto sr = compiler.get_shader_resources();
			//for (auto& in : sr.stage_inputs) compiler.set_name(in.id, in.name.substr(7));
			//for (auto& in : sr.stage_outputs) compiler.set_name(in.id, in.name.substr(8));

			compiler.build_combined_image_samplers();
			for (auto& remap : compiler.get_combined_image_samplers()) {
				auto& texName = compiler.get_name(remap.image_id);
				compiler.set_name(remap.combined_id, "_CombinedTex" + String::toString(texName.size()) + "_" + texName + compiler.get_name(remap.sampler_id));
			}

			try {
				auto str = compiler.compile();

				dst.language = targetLanguage;
				dst.stage = source.stage;
				dst.version = String::toString(opts.version);
				dst.data.setCapacity(str.size());
				dst.data.writeBytes((uint8_t*)str.c_str(), str.size());
			} catch (spirv_cross::CompilerError& error) {
				println("spirv to glsl/gssl error : ", error.what());
			}

			break;
		}
		case ProgramLanguage::MSL:
		{
			spirv_cross::CompilerMSL compiler((uint32_t*)sourceData, sourceDataSize / sizeof(uint32_t));

			compiler.set_entry_point(ProgramSource::getEntryPoint(source), model);

			auto opts = compiler.get_common_options();
			if (!targetVersion.empty()) opts.version = std::stoi(targetVersion);
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
			if (!targetVersion.empty()) mslOpts.msl_version = std::stoi(targetVersion);
			mslOpts.swizzle_texture_samples = false;
			compiler.set_msl_options(mslOpts);

			try {
				auto str = compiler.compile();

				dst.language = targetLanguage;
				dst.stage = source.stage;
				dst.version = String::toString(mslOpts.msl_version);
				dst.data.setCapacity(str.size());
				dst.data.writeBytes((uint8_t*)str.c_str(), str.size());
			} catch (spirv_cross::CompilerError& error) {
				println(error.what());
			}

			break;
		}
		default:
			break;
		}
	}
}