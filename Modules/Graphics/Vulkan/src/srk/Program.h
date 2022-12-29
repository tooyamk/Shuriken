#pragma once

#include "Base.h"
#include "srk/modules/graphics/ConstantBufferManager.h"
#include "spirv_reflect.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) override;
		virtual const ProgramInfo& getInfo() const override;
		virtual void SRK_CALL destroy() override;

		inline const std::vector<VkPipelineShaderStageCreateInfo>& getVkPipelineShaderStageCreateInfos() const {
			return _createInfos;
		}

		inline VkPipelineLayout getVkPipelineLayout() const {
			return _pipelineLayout;
		}

		bool SRK_CALL use(const IVertexAttributeGetter* vertexAttributeGetter, 
			VkVertexInputBindingDescription* vertexInputBindingDescs, uint32_t& vertexInputBindingDescsCount,
			VkVertexInputAttributeDescription* vertexInputAttribDescs, uint32_t& vertexInputAttribDescCount);

	protected:
		inline static constexpr std::string_view inputNamePrefix = std::string_view("in.var.");


		struct VertexLayout {
			uint32_t location;
		};


		struct TextureLayout {
			std::string name;
			uint32_t bindPoint = 0;
		};


		struct SamplerLayout {
			std::string name;
			uint32_t bindPoint = 0;
		};


		struct ParameterLayout {
			std::vector<ConstantBufferLayout> constantBuffers;
			std::vector<TextureLayout> textures;
			std::vector<SamplerLayout> samplers;

			void clear(Graphics& g);
		};


		std::vector<std::string> _entryPoints;
		std::vector<VkPipelineShaderStageCreateInfo> _createInfos;
		VkDescriptorSetLayout _descriptorSetLayout;
		VkDescriptorPool _descriptorPool;
		VkDescriptorSet _descriptorSet;
		VkPipelineLayout _pipelineLayout;

		std::vector<VertexLayout> _vertexLayouts;

		bool _valid;
		ProgramInfo _info;

		ParameterLayout _vsParamLayout;
		ParameterLayout _psParamLayout;

		bool SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler, std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
		void SRK_CALL _reflect(const SpvReflectShaderModule* data, const InputHandler& inputHandler, std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
		void SRK_CALL _parseParamLayout(const SpvReflectShaderModule* data, ParameterLayout& layout, VkShaderStageFlags stageFlags, std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
		void SRK_CALL _parseParamLayout(const SpvReflectTypeDescription* data, struct SpvReflectBlockVariable* members, std::vector<ConstantBufferLayout::Variables>& vars);
	};
}