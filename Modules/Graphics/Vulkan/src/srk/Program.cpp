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
		for (auto& set : sets) {
			for (auto& binding : set.bindings) {
				if (binding.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) continue;
				g.getConstantBufferManager().unregisterConstantLayout(binding.data);
			}
		}
		sets.clear();
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

		std::vector<std::vector<SetLayout>*> layouts = { &_vsParamLayout.sets, &_psParamLayout.sets };
		_calcConstantLayoutSameBuffers(layouts);

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

		_vsParamLayout.clear(g);
		_psParamLayout.clear(g);

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
			_useParameters(_vsParamLayout, *shaderParamGetter);
			_useParameters(_psParamLayout, *shaderParamGetter);
		}

		return true;
	}

	void Program::useEnd() {
		for (auto& i : _usingSameConstBuffers) i = nullptr;
	}

	bool Program::_compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings) {
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

	void Program::_parseParamLayout(const SpvReflectShaderModule* data, ParameterLayout& layout, VkShaderStageFlags stageFlags, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings) {
		layout.sets.resize(data->descriptor_set_count);

		for (decltype(data->descriptor_set_count) i = 0; i < data->descriptor_set_count; ++i) {
			auto& refSet = data->descriptor_sets[i];
			auto& set = layout.sets[i];
			set.index = refSet.set;
			set.constantBufferCount = 0;
			auto& descSetLayoutBindings = descriptorSetLayoutBindings.emplace_back();
			
			for (decltype(refSet.binding_count) j = 0; j < refSet.binding_count; ++j) {
				auto refBinding = refSet.bindings[j];
				auto& binding = set.bindings.emplace_back();

				binding.setName(refBinding->name);
				binding.bindPoint = refBinding->binding;

				VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
				descriptorSetLayoutBinding.binding = refBinding->binding;
				descriptorSetLayoutBinding.descriptorCount = 0;
				descriptorSetLayoutBinding.stageFlags = stageFlags;
				descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

				switch (refBinding->descriptor_type) {
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
				{
					binding.type = VK_DESCRIPTOR_TYPE_SAMPLER;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
					descriptorSetLayoutBinding.descriptorCount = 1;

					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				{
					binding.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
					descriptorSetLayoutBinding.descriptorCount = 1;

					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					binding.data.size = refBinding->block.size;

					descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

					_parseParamLayout(refBinding->type_description, refBinding->block.members, binding.data.variables, descriptorSetLayoutBinding);

					if (descriptorSetLayoutBinding.descriptorCount) {
						binding.data.calcFeatureValue();
						++set.constantBufferCount;

						_graphics.get<Graphics>()->getConstantBufferManager().registerConstantLayout(binding.data);
					}

					break;
				}
				default:
					break;
				}

				if (descriptorSetLayoutBinding.descriptorCount) {
					descSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);
				} else {
					set.bindings.pop_back();
				}
			}
		}
	}

	void Program::_parseParamLayout(const SpvReflectTypeDescription* data, struct SpvReflectBlockVariable* members, std::vector<ConstantBufferLayout::Variables>& vars, VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding) {
		using namespace srk::literals;

		if (!data) return;

		for (decltype(data->member_count) i = 0; i < data->member_count; ++i) {
			auto& memDesc = data->members[i];
			auto& blockVar = members[i];
			auto& v = vars.emplace_back();
			if (memDesc.storage_class == SpvStorageClassUniformConstant) {
				v.name = memDesc.struct_member_name;
				++descriptorSetLayoutBinding.descriptorCount;
			}
			v.offset = blockVar.offset;
			v.size = blockVar.size;
			v.stride = (1_ui32 << 31) | 16;

			_parseParamLayout(&memDesc, blockVar.members, v.members, descriptorSetLayoutBinding);
		}
	}

	void Program::_calcConstantLayoutSameBuffers(std::vector<std::vector<SetLayout>*>& setLayouts) {
		uint32_t sameId = 0;
		auto n = setLayouts.size();
		for (size_t i = 0; i < n; ++i) {
			auto& sets0 = *setLayouts[i];
			for (size_t j = 0; j < n; ++j) {
				if (i == j) continue;

				auto& sets1 = *setLayouts[j];

				for (auto& set0 : sets0) {
					if (!set0.constantBufferCount) continue;

					for (auto& binding0 : set0.bindings) {
						if (binding0.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) continue;

						for (auto& set1 : sets1) {
							if (!set1.constantBufferCount) continue;

							for (auto& binding1 : set1.bindings) {
								if (binding1.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) continue;

								if (binding0.data.featureValue == binding1.data.featureValue) {
									if (binding0.data.sameId == 0) {
										binding0.data.sameId = ++sameId;
										_usingSameConstBuffers.emplace_back(nullptr);
									}
									binding1.data.sameId = binding0.data.sameId;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	ConstantBuffer* Program::_getConstantBuffer(const MyConstantBufferLayout& cbLayout, const IShaderParameterGetter& paramGetter) {
		if (cbLayout.sameId) {
			if (auto cb = _usingSameConstBuffers[cbLayout.sameId - 1]; cb) return cb;
		}

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
		if (cb && cbLayout.sameId) _usingSameConstBuffers[cbLayout.sameId - 1] = cb;

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

	void Program::_useParameters(const ParameterLayout& layout, const IShaderParameterGetter& paramGetter) {
		/*auto g = _graphics.get<Graphics>();

		for (auto& info : layout.constantBuffers) {
			auto cb = _getConstantBuffer(info, paramGetter);
			if (cb && g == cb->getGraphics()) {
				if (auto native = (BaseBuffer*)cb->getNative(); native) {
					if (auto buffer = native->getBuffer(); buffer) {
						VkDescriptorBufferInfo descriptorBufferInfo;
						descriptorBufferInfo.buffer = buffer;
						descriptorBufferInfo.offset = 0;
						descriptorBufferInfo.range = info.size;

						VkWriteDescriptorSet writeDescriptorSet;
						memset(&writeDescriptorSet, 0, sizeof(writeDescriptorSet));
						writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						writeDescriptorSet.dstSet = _descriptorSet;
						writeDescriptorSet.dstBinding = info.bindPoint;
						writeDescriptorSet.dstArrayElement = 0;
						writeDescriptorSet.descriptorCount = 1;
						writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						writeDescriptorSet.pImageInfo = nullptr;
						writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
						writeDescriptorSet.pTexelBufferView = nullptr;

						vkUpdateDescriptorSets(g->getDevice(), 1, &writeDescriptorSet, 0, nullptr);
					}
				}
			}
		}*/
	}
}