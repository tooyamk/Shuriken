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
		textures.clear();
		samplers.clear();
	}


	Program::Program(Graphics& graphics) : IProgram(graphics),
		_descriptorPool(nullptr),
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

		std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;

		if (!_compileShader(vert, ProgramStage::VS, 0, defines, numDefines, includeHandler, inputHandler, descriptorSetLayoutBindings)) {
			destroy();
			return false;
		}

		if (!_compileShader(frag, ProgramStage::PS, _calcSet0BingingOffset(descriptorSetLayoutBindings), defines, numDefines, includeHandler, inputHandler, descriptorSetLayoutBindings)) {
			destroy();
			return false;
		}

		auto g = _graphics.get<Graphics>();
		auto device = g->getDevice();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		memset(&descriptorSetLayoutCreateInfo, 0, sizeof(descriptorSetLayoutCreateInfo));
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		for (size_t i = 0; i < descriptorSetLayoutBindings.size(); ++i) {
			descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings[i].size();
			descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings[i].data();
			VkDescriptorSetLayout layout;
			if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, g->getVkAllocationCallbacks(), &layout) != VK_SUCCESS) {
				destroy();
				return false;
			}

			_descriptorSetLayouts.emplace_back(layout);
		}
		

		VkPipelineLayoutCreateInfo pipelineLayoutInfo;
		memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = _descriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = _descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, g->getVkAllocationCallbacks(), &_pipelineLayout) != VK_SUCCESS) {
			destroy();
			return false;
		}

		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		for (const auto& i : descriptorSetLayoutBindings) {
			for (const auto& j : i) {
				auto found = false;
				for (auto& k : descriptorPoolSizes) {
					if (j.descriptorType == k.type) {
						k.descriptorCount += j.descriptorCount;
						found = true;
						break;
					}
				}

				if (!found) {
					auto& dps = descriptorPoolSizes.emplace_back();
					dps.type = j.descriptorType;
					dps.descriptorCount = j.descriptorCount;
				}
			}
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		memset(&descriptorPoolCreateInfo, 0, sizeof(descriptorPoolCreateInfo));
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.maxSets = _descriptorSetLayouts.size();
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
		descriptorSetAllocateInfo.descriptorSetCount = _descriptorSetLayouts.size();
		descriptorSetAllocateInfo.pSetLayouts = _descriptorSetLayouts.data();
		_descriptorSets.resize(_descriptorSetLayouts.size());
		if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, _descriptorSets.data()) != VK_SUCCESS) {
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
		}
		if (_pipelineLayout) {
			vkDestroyPipelineLayout(device, _pipelineLayout, g.getVkAllocationCallbacks());
			_pipelineLayout = nullptr;
		}
		for (auto& i : _descriptorSetLayouts) vkDestroyDescriptorSetLayout(device, i, g.getVkAllocationCallbacks());
		for (auto& i : _createInfos) vkDestroyShaderModule(device, i.module, g.getVkAllocationCallbacks());

		_descriptorSetLayouts.clear();
		_descriptorSets.clear();
		_createInfos.clear();
		_entryPoints.clear();

		_info.clear();
		_vertexLayouts.clear();

		_paramLayout.clear(g);

		_valid = false;
	}

	bool Program::use(const IVertexAttributeGetter* vertexAttributeGetter, 
		VkVertexInputBindingDescription* vertexInputBindingDescs, uint32_t& vertexInputBindingDescsCount,
		VkVertexInputAttributeDescription* vertexInputAttribDescs, uint32_t& vertexInputAttribDescCount,
		const IShaderParameterGetter* shaderParamGetter) {
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

		if (shaderParamGetter) {
			std::vector<VkDescriptorBufferInfo> descriptorBufferInfos;
			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			for (auto& b : _paramLayout.constantBuffers) {
				auto cb = _getConstantBuffer(b, *shaderParamGetter);
				if (cb && _graphics == cb->getGraphics()) {
					if (auto native = (BaseBuffer*)cb->getNative(); native) {
						if (auto buffer = native->getBuffer(); buffer) {
							auto& writeDescriptorSet = writeDescriptorSets.emplace_back();
							writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
							writeDescriptorSet.pNext = nullptr;
							writeDescriptorSet.dstSet = _descriptorSets[b.bindingInfo.set];
							writeDescriptorSet.dstBinding = b.bindingInfo.binding;
							writeDescriptorSet.dstArrayElement = 0;
							writeDescriptorSet.descriptorCount = 1;
							writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
							writeDescriptorSet.pImageInfo = nullptr;
							writeDescriptorSet.pBufferInfo = (VkDescriptorBufferInfo*)descriptorBufferInfos.size();
							writeDescriptorSet.pTexelBufferView = nullptr;

							auto& descriptorBufferInfo = descriptorBufferInfos.emplace_back();
							descriptorBufferInfo.buffer = buffer;
							descriptorBufferInfo.offset = 0;
							descriptorBufferInfo.range = b.size;
						}
					}
				}
			}

			for (auto& i : writeDescriptorSets) {
				if (i.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) i.pBufferInfo = &descriptorBufferInfos[(size_t)i.pBufferInfo];
			}

			if (writeDescriptorSets.size()) vkUpdateDescriptorSets(_graphics.get<Graphics>()->getDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}

		return true;
	}

	uint32_t Program::_calcSet0BingingOffset(const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings) {
		if (descriptorSetLayoutBindings.empty()) return 0;

		uint32_t offset = 0;
		for (auto& i : descriptorSetLayoutBindings[0]) {
			if (offset <= i.binding) offset = i.binding + 1;
		}
		return offset;
	}

	bool Program::_compileShader(const ProgramSource& source, ProgramStage stage, uint32_t set0BindingOffset, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings) {
		if (!source.isValid()) {
			_graphics.get<Graphics>()->error("spirv compile failed, source is invalid");
			return false;
		}

		auto g = _graphics.get<Graphics>();

		if (source.language != ProgramLanguage::SPIRV) {
			if (auto transpiler = g->getShaderTranspiler(); transpiler) {
				IShaderTranspiler::Options options;
				options.spirv.descriptorSet0BindingOffset = set0BindingOffset;
				return _compileShader(transpiler->translate(source, options, ProgramLanguage::SPIRV, "", defines, numDefines, [this, stage, &includeHandler](const std::string_view& name) {
					if (includeHandler) return includeHandler(*this, stage, name);
					return ByteArray();
					}), stage, set0BindingOffset, defines, numDefines, includeHandler, inputHandler, descriptorSetLayoutBindings);
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
			_parse(&reflectModule, inputHandler, descriptorSetLayoutBindings);
			spvReflectDestroyShaderModule(&reflectModule);
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

	void Program::_parse(const SpvReflectShaderModule* data, const InputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings) {
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

			_parseParamLayout(data, VK_SHADER_STAGE_VERTEX_BIT, descriptorSetLayoutBindings);

			break;
		}
		case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		{
			_parseParamLayout(data, VK_SHADER_STAGE_FRAGMENT_BIT, descriptorSetLayoutBindings);
			break;
		}
		}
	}

	void Program::_parseParamLayout(const SpvReflectShaderModule* data, VkShaderStageFlags stageFlags, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings) {
		for (decltype(data->descriptor_set_count) i = 0; i < data->descriptor_set_count; ++i) {
			auto& refSet = data->descriptor_sets[i];

			if (descriptorSetLayoutBindings.size() < refSet.set + 1) {
				for (size_t i = 0, n = refSet.set + 1 - descriptorSetLayoutBindings.size(); i < n; ++i) descriptorSetLayoutBindings.emplace_back();
			}
			auto& descSetLayoutBindings = descriptorSetLayoutBindings[refSet.set];
			
			for (decltype(refSet.binding_count) j = 0; j < refSet.binding_count; ++j) {
				auto refBinding = refSet.bindings[j];

				BindingInfo bi;
				bi.set = refSet.set;
				bi.binding = refBinding->binding;

				VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
				descriptorSetLayoutBinding.binding = refBinding->binding;
				descriptorSetLayoutBinding.descriptorCount = 1;
				descriptorSetLayoutBinding.stageFlags = stageFlags;
				descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

				switch (refBinding->descriptor_type) {
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
				{
					auto& s = _paramLayout.samplers.emplace_back();
					s.name = refBinding->name;
					s.bindingInfo = bi;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				{
					auto& t = _paramLayout.textures.emplace_back();
					t.name = refBinding->name;
					t.bindingInfo = bi;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					auto& b = _paramLayout.constantBuffers.emplace_back();
					b.name = refBinding->name;
					b.size = refBinding->block.size;
					b.bindingInfo = bi;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					_parseParamLayout(refBinding->type_description, refBinding->block.members, b.variables);

					b.calcFeatureValue();

					_graphics.get<Graphics>()->getConstantBufferManager().registerConstantLayout(b);

					break;
				}
				default:
					descriptorSetLayoutBinding.descriptorCount = 0;
					break;
				}

				if (descriptorSetLayoutBinding.descriptorCount) descSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);
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

	ConstantBuffer* Program::_getConstantBuffer(const MyConstantBufferLayout& cbLayout, const IShaderParameterGetter& paramGetter) {
		ShaderParameterUsageStatistics statistics;
		cbLayout.collectUsingInfo(paramGetter, statistics, (std::vector<const ShaderParameter*>&)_tempParams, _tempVars);

		ConstantBuffer* cb = nullptr;
		if (decltype(statistics.unknownCount) numVars = _tempVars.size(); statistics.unknownCount < numVars) {
			auto g = _graphics.get<Graphics>();

			if (statistics.exclusiveCount && !statistics.shareCount) {
				if (cb = (ConstantBuffer*)g->getConstantBufferManager().getExclusiveConstantBuffer(_tempParams, cbLayout); cb) {
					auto isMaping = false;
					for (decltype(numVars) i = 0; i < numVars; ++i) {
						if (auto param = _tempParams[i]; param && param->getUpdateId() != cb->recordUpdateIds[i]) {
							if (!isMaping) {
								if (cb->map(Usage::MAP_WRITE) == Usage::NONE) break;
								isMaping = true;
							}
							cb->recordUpdateIds[i] = param->getUpdateId();
							ConstantBufferManager::updateConstantBuffer(cb, *param, *_tempVars[i]);
						}
					}

					if (isMaping) cb->unmap();
				}
			} else {
				if (cb = (ConstantBuffer*)g->getConstantBufferManager().popShareConstantBuffer(cbLayout.size); cb) _constantBufferUpdateAll(cb, cbLayout.variables);
			}
		}

		_tempParams.clear();
		_tempVars.clear();

		return cb;
	}

	void Program::_constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars) {
		if (cb->map(Usage::MAP_WRITE) != Usage::NONE) {
			for (size_t i = 0, n = _tempVars.size(); i < n; ++i) {
				if (auto param = _tempParams[i]; param) ConstantBufferManager::updateConstantBuffer(cb, *param, *_tempVars[i]);
			}

			cb->unmap();
		}
	}
}