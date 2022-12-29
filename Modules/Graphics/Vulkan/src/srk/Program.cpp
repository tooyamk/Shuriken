#include "Program.h"
#include "Graphics.h"
#include "VertexBuffer.h"
#include "srk/GraphicsBuffer.h"
#include "srk/ProgramSource.h"
#include "srk/ShaderDefine.h"
#include "srk/ShaderParameter.h"
#include "srk/String.h"
#include <vector>
#include "srk/Debug.h"

namespace srk::modules::graphics::vulkan {
	void Program::ParameterLayout::clear(Graphics& g) {
		for (auto& buffer : constantBuffers) g.getConstantBufferManager().unregisterConstantLayout(buffer);
		constantBuffers.clear();
	}


	Program::Program(Graphics& graphics) : IProgram(graphics),
		_descriptorSetLayout(nullptr),
		_descriptorPool(nullptr),
		_descriptorSet(nullptr),
		_pipelineLayout(nullptr),
		_valid(false) {
	}

	Program::~Program() {
		destroy();
	}

	const void* Program::getNative() const {
		return this;
	}

	bool Program::create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) {
		destroy();

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

		if (!_compileShader(vert, ProgramStage::VS, defines, numDefines, includeHandler, inputHandler, descriptorSetLayoutBindings)) {
			destroy();
			return false;
		}

		if (!_compileShader(frag, ProgramStage::PS, defines, numDefines, includeHandler, inputHandler, descriptorSetLayoutBindings)) {
			destroy();
			return false;
		}

		auto g = _graphics.get<Graphics>();
		auto device = g->getDevice();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		memset(&descriptorSetLayoutCreateInfo, 0, sizeof(descriptorSetLayoutCreateInfo));
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
		if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, g->getVkAllocationCallbacks(), &_descriptorSetLayout) != VK_SUCCESS) {
			destroy();
			return false;
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo;
		memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, g->getVkAllocationCallbacks(), &_pipelineLayout) != VK_SUCCESS) {
			destroy();
			return false;
		}

		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		for (const auto& itr : descriptorSetLayoutBindings) {
			auto found = false;
			for (auto& itr2 : descriptorPoolSizes) {
				if (itr.descriptorType == itr2.type) {
					itr2.descriptorCount += itr.descriptorCount;
					found = true;
					break;
				}
			}

			if (!found) {
				auto& dps = descriptorPoolSizes.emplace_back();
				dps.type = itr.descriptorType;
				dps.descriptorCount = 1;
			}
		}
		uint32_t maxSets = 0;
		for (auto& itr : descriptorPoolSizes) maxSets += itr.descriptorCount;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		memset(&descriptorPoolCreateInfo, 0, sizeof(descriptorPoolCreateInfo));
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.maxSets = maxSets;
		descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
		if (vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, g->getVkAllocationCallbacks(), &_descriptorPool) != VK_SUCCESS) {
			destroy();
			return false;
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		memset(&descriptorSetAllocateInfo, 0, sizeof(descriptorSetAllocateInfo));
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &_descriptorSetLayout;
		if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &_descriptorSet) != VK_SUCCESS) {
			destroy();
			return false;
		}

		for (size_t i = 0; i < _createInfos.size(); ++i) _createInfos[i].pName = _entryPoints[i].data();

		_valid = true;

		return true;
	}

	const ProgramInfo& Program::getInfo() const {
		return _info;
	}

	void Program::destroy() {
		auto& g = *_graphics.get<Graphics>();
		auto device = g.getDevice();

		if (_descriptorPool) {
			vkDestroyDescriptorPool(device, _descriptorPool, g.getVkAllocationCallbacks());
			_descriptorPool = nullptr;
			_descriptorSet = nullptr;
		}
		if (_pipelineLayout) {
			vkDestroyPipelineLayout(device, _pipelineLayout, g.getVkAllocationCallbacks());
			_pipelineLayout = nullptr;
		}
		if (_descriptorSetLayout) {
			vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, g.getVkAllocationCallbacks());
			_descriptorSetLayout = nullptr;
		}
		for (auto& i : _createInfos) vkDestroyShaderModule(device, i.module, g.getVkAllocationCallbacks());

		_createInfos.clear();
		_entryPoints.clear();

		_info.clear();
		_vertexLayouts.clear();

		_vsParamLayout.clear(g);
		_psParamLayout.clear(g);

		_valid = false;
	}

	bool Program::use(const IVertexAttributeGetter* vertexAttributeGetter, 
		VkVertexInputBindingDescription* vertexInputBindingDescs, uint32_t& vertexInputBindingDescsCount,
		VkVertexInputAttributeDescription* vertexInputAttribDescs, uint32_t& vertexInputAttribDescCount) {
		if (!_valid) return false;

		auto n = _info.vertices.size();
		if (n && (!vertexAttributeGetter || vertexInputAttribDescCount < n)) return false;

		std::unordered_map<void*, uint32_t> bindings;
		uint32_t vertexInputBindingDescsCnt = 0;
		vertexInputAttribDescCount = n;
		for (size_t i = 0; i < n; ++i) {
			auto& v = _info.vertices[i];

			auto opt = vertexAttributeGetter->get(v.name);
			if (!opt) return false;

			auto& va = *opt;
			if (!va.resource || va.resource->getGraphics() != _graphics)  return false;

			auto stride = va.resource->getStride();
			if (!stride) return false;

			auto native = (const BaseBuffer*)va.resource->getNative();
			if (!native) return false;

			auto buf = native->getBuffer();
			if (!buf) return false;

			auto& desc = va.desc;
			auto fmt = Graphics::convertVertexFormat(desc.format);
			if (fmt == VK_FORMAT_UNDEFINED) return false;

			uint32_t binding;
			auto bindingRst = bindings.try_emplace(buf, bindings.size());
			if (bindingRst.second) {
				if (vertexInputBindingDescsCnt >= vertexInputBindingDescsCount) return false;

				auto& vbd = vertexInputBindingDescs[vertexInputBindingDescsCnt];
				vbd.binding = vertexInputBindingDescsCnt;
				vbd.stride = stride;
				vbd.inputRate = v.instanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

				binding = vertexInputBindingDescsCnt;
				++vertexInputBindingDescsCnt;
			} else {
				binding = bindingRst.first->second;
			}

			auto& vad = vertexInputAttribDescs[i];
			vad.location = _vertexLayouts[i].location;
			vad.binding = binding;
			vad.format = fmt;
			vad.offset = desc.offset;
		}

		vertexInputBindingDescsCount = vertexInputBindingDescsCnt;

		return true;
	}

	bool Program::_compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler, std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings) {
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
					}), stage, defines, numDefines, includeHandler, inputHandler, descriptorSetLayoutBindings);
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
			memset(&shaderModuleCreateInfo, 0, sizeof(shaderModuleCreateInfo));
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = ba.getLength();
			shaderModuleCreateInfo.pCode = (const uint32_t*)ba.getSource();

			if (vkCreateShaderModule(g->getDevice(), &shaderModuleCreateInfo, g->getVkAllocationCallbacks(), &shaderModule) != VK_SUCCESS) return false;

			SpvReflectShaderModule reflectModule;
			if (spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NO_COPY, ba.getLength(), ba.getSource(), &reflectModule) != SPV_REFLECT_RESULT_SUCCESS) return false;
			_reflect(&reflectModule, inputHandler, descriptorSetLayoutBindings);
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

	void Program::_reflect(const SpvReflectShaderModule* data, const InputHandler& inputHandler, std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings) {
		switch (data->shader_stage) {
		case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
		{
			_info.clear();
			_vertexLayouts.resize(data->input_variable_count);
			_info.vertices.resize(data->input_variable_count);

			for (decltype(data->input_variable_count) i = 0; i < data->input_variable_count; ++i) {
				auto iv = data->input_variables[i];
				auto& vd = _info.vertices[i];
				vd.name = std::string_view(iv->name + inputNamePrefix.size());

				auto& vl = _vertexLayouts[i];
				vl.location = iv->location;

				switch (iv->format) {
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

			_parseParamLayout(data, _vsParamLayout, VK_SHADER_STAGE_VERTEX_BIT, descriptorSetLayoutBindings);

			break;
		}
		case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		{
			_parseParamLayout(data, _psParamLayout, VK_SHADER_STAGE_FRAGMENT_BIT, descriptorSetLayoutBindings);
			break;
		}
		}
	}

	void Program::_parseParamLayout(const SpvReflectShaderModule* data, ParameterLayout& layout, VkShaderStageFlags stageFlags, std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings) {
		for (decltype(data->descriptor_set_count) i = 0; i < data->descriptor_set_count; ++i) {
			auto& descSet = data->descriptor_sets[i];

			for (decltype(descSet.binding_count) j = 0; j < descSet.binding_count; ++j) {
				auto descBinding = descSet.bindings[j];

				VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
				descriptorSetLayoutBinding.binding = descBinding->binding;
				descriptorSetLayoutBinding.descriptorCount = 1;
				descriptorSetLayoutBinding.stageFlags = stageFlags;
				descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

				switch (descBinding->descriptor_type) {
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
				{
					auto& sampler = layout.samplers.emplace_back();
					sampler.name = descBinding->name;
					sampler.bindPoint = descBinding->binding;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				{
					auto& texture = layout.textures.emplace_back();
					texture.name = descBinding->name;
					texture.bindPoint = descBinding->binding;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					auto& cb = layout.constantBuffers.emplace_back();
					cb.name = descBinding->name;
					cb.bindPoint = descBinding->binding;
					cb.size = descBinding->block.size;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

					_parseParamLayout(descBinding->type_description, descBinding->block.members, cb.variables);

					break;
				}
				default:
					break;
				}

				auto found = false;
				for (auto& itr : descriptorSetLayoutBindings) {
					if (itr.binding == descriptorSetLayoutBinding.binding && itr.descriptorType == descriptorSetLayoutBinding.descriptorType) {
						itr.stageFlags |= stageFlags;
						found = true;
						break;
					}
				}

				if (!found) descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);
			}
		}
	}

	void Program::_parseParamLayout(const SpvReflectTypeDescription* data, struct SpvReflectBlockVariable* members, std::vector<ConstantBufferLayout::Variables>& vars) {
		using namespace srk::literals;

		if (!data) return;

		for (decltype(data->member_count) i = 0; i < data->member_count; ++i) {
			auto& memDesc = data->members[i];
			auto& blockVar = members[i];
			auto& v = vars.emplace_back();
			if (memDesc.storage_class == SpvStorageClassUniformConstant) {
				v.name = memDesc.struct_member_name;
			}
			v.offset = blockVar.offset;
			v.size = blockVar.size;
			v.stride = (1_ui32 << 31) | 16;

			_parseParamLayout(&memDesc, blockVar.members, v.members);
		}
	}
}