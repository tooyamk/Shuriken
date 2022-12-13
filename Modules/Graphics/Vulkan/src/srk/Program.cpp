#include "Program.h"
#include "Graphics.h"
#include "srk/GraphicsBuffer.h"
#include "srk/ProgramSource.h"
#include "srk/ShaderDefine.h"
#include "srk/ShaderParameter.h"
#include "srk/String.h"
#include "spirv_reflect.h"
#include <vector>
#include "srk/Debug.h"

namespace srk::modules::graphics::vulkan {
	Program::Program(Graphics& graphics) : IProgram(graphics) {
	}

	Program::~Program() {
		destroy();
	}

	const void* Program::getNative() const {
		return this;
	}

	bool Program::create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) {
		destroy();

		if (!_compileShader(vert, ProgramStage::VS, defines, numDefines, includeHandler, inputHandler)) {
			destroy();
			return false;
		}

		if (!_compileShader(frag, ProgramStage::PS, defines, numDefines, includeHandler, inputHandler)) {
			destroy();
			return false;
		}

		for (size_t i = 0; i < _createInfos.size(); ++i) _createInfos[i].pName = _entryPoints[i].data();

		return true;
	}

	const ProgramInfo& Program::getInfo() const {
		return _info;
	}

	void Program::destroy() {
		for (auto& i : _createInfos) vkDestroyShaderModule(_graphics.get<Graphics>()->getDevice(), i.module, nullptr);
		_createInfos.clear();
		_entryPoints.clear();

		_info.clear();
	}

	bool Program::_compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) {
		if (!source.isValid()) {
			_graphics.get<Graphics>()->error("spirv compile failed, source is invalid");
			return false;
		}

		auto g = _graphics.get<Graphics>();

		if (source.language != ProgramLanguage::SPIRV) {
			if (auto transpiler = g->getShaderTranspiler(); transpiler) {
				return _compileShader(transpiler->translate(source, ProgramLanguage::SPIRV, "", defines, numDefines, [this, stage, &includeHandler](const std::string_view& name) {
					if (includeHandler) return includeHandler(*this, stage, name);
					return ByteArray();
					}), stage, defines, numDefines, includeHandler, inputHandler);
			} else {
				return false;
			}
		}

		VkShaderModule shaderModule;
		{
			ByteArray ba;
			auto rem = source.data.getLength() & 0b11;
			if (rem == 0) {
				ba = ByteArray((void*)source.data.getSource(), source.data.getLength());
			} else {
				ba.setCapacity(source.data.getLength() + rem);
				ba.write<ba_vt::BYTE>(source.data);
				for (decltype(rem) i = 0; i < rem; ++i) ba.write<uint8_t>(0);
			}

			VkShaderModuleCreateInfo shaderModuleCreateInfo;
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.pNext = nullptr;
			shaderModuleCreateInfo.flags = 0;
			shaderModuleCreateInfo.codeSize = ba.getLength();
			shaderModuleCreateInfo.pCode = (const uint32_t*)ba.getSource();

			if (vkCreateShaderModule(g->getDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) return false;

			SpvReflectShaderModule reflectModule;
			if (spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NO_COPY, ba.getLength(), ba.getSource(), &reflectModule) != SPV_REFLECT_RESULT_SUCCESS) return false;
			_reflect(&reflectModule, inputHandler);
			spvReflectDestroyShaderModule(&reflectModule);
			int a = 1;
		}

		auto& entry = _entryPoints.emplace_back(source.getEntryPoint());
		auto& info = _createInfos.emplace_back();
		memset(&info, 0, sizeof(info));
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage = Graphics::convertProgramStage(stage);
		info.module = shaderModule;
		info.pSpecializationInfo = nullptr;

		return true;
	}

	void Program::_reflect(const void* data, const InputHandler& inputHandler) {
		auto reflectData = (const SpvReflectShaderModule*)data;
		switch (reflectData->shader_stage) {
		case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
		{
			_info.clear();

			for (decltype(reflectData->interface_variable_count) i = 0; i < reflectData->interface_variable_count; ++i) {
				auto& iv = reflectData->interface_variables[i];
				if (iv.storage_class == SpvStorageClassInput) {
					auto& vd = _info.vertices.emplace_back();
					vd.name = std::string_view(iv.name + InputNameSkipLength);
					
					switch (iv.format) {
					case SPV_REFLECT_FORMAT_R32_SINT:
						vd.format.set(1, VertexType::I32);
						break;
					case SPV_REFLECT_FORMAT_R32_UINT:
						vd.format.set(1, VertexType::UI32);
						break;
					case SPV_REFLECT_FORMAT_R32_SFLOAT:
						vd.format.set(1, VertexType::F32);
						break;
					case SPV_REFLECT_FORMAT_R32G32_SINT:
						vd.format.set(2, VertexType::I32);
						break;
					case SPV_REFLECT_FORMAT_R32G32_UINT:
						vd.format.set(2, VertexType::UI32);
						break;
					case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
						vd.format.set(2, VertexType::F32);
						break;
					case SPV_REFLECT_FORMAT_R32G32B32_SINT:
						vd.format.set(3, VertexType::I32);
						break;
					case SPV_REFLECT_FORMAT_R32G32B32_UINT:
						vd.format.set(3, VertexType::UI32);
						break;
					case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
						vd.format.set(3, VertexType::F32);
						break;
					case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
						vd.format.set(4, VertexType::I32);
						break;
					case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
						vd.format.set(4, VertexType::UI32);
						break;
					case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
						vd.format.set(4, VertexType::F32);
						break;
					}

					auto inputDesc = inputHandler ? inputHandler(*this, vd.name) : InputDescriptor();
					vd.instanced = inputDesc.instanced;
				}
			}

			break;
		}
		}
	}
}