#pragma once

#include "ConstantBuffer.h"
#include "srk/modules/graphics/ConstantBufferManager.h"
#include "spirv_reflect.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ProgramDefine* defines, size_t numDefines, const ProgramIncludeHandler& includeHandler, const ProgramInputHandler& inputHandler, const ProgramTranspileHandler& transpileHandler) override;
		virtual const ProgramInfo& getInfo() const override;
		virtual void SRK_CALL destroy() override;

		inline const std::vector<VkPipelineShaderStageCreateInfo>& getVkPipelineShaderStageCreateInfos() const {
			return _createInfos;
		}

		inline VkPipelineLayout getVkPipelineLayout() const {
			return _pipelineLayout;
		}

		inline const std::vector<VkDescriptorSet>& getVkDescriptorSets() const {
			return _descriptorSets;
		}

		bool SRK_CALL use(const IVertexAttributeGetter* vertexAttributeGetter, 
			VkVertexInputBindingDescription* vertexInputBindingDescs, uint32_t& vertexInputBindingDescsCount,
			VkVertexInputAttributeDescription* vertexInputAttribDescs, uint32_t& vertexInputAttribDescCount,
			const IShaderParameterGetter* shaderParamGetter);

	protected:
		inline static constexpr std::string_view inputNamePrefix = std::string_view("in.var.");


		struct VertexLayout {
			uint32_t location;
		};


		struct BindingInfo {
			uint32_t set = 0;
			uint32_t binding = 0;
		};


		class MyConstantBufferLayout : public ConstantBufferLayout {
		public:
			BindingInfo bindingInfo;
		};


		struct TextureLayout {
			std::string name;
			BindingInfo bindingInfo;
		};


		struct SamplerLayout {
			std::string name;
			BindingInfo bindingInfo;
		};


		struct ParameterLayout {
			std::vector<MyConstantBufferLayout> constantBuffers;
			std::vector<TextureLayout> textures;
			std::vector<SamplerLayout> samplers;

			void clear(Graphics& g);
		};


		std::vector<std::string> _entryPoints;
		std::vector<VkPipelineShaderStageCreateInfo> _createInfos;
		std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
		VkDescriptorPool _descriptorPool;
		std::vector<VkDescriptorSet> _descriptorSets;
		VkPipelineLayout _pipelineLayout;

		std::vector<VertexLayout> _vertexLayouts;

		bool _valid;
		ProgramInfo _info;

		ParameterLayout _paramLayout;

		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		static uint32_t SRK_CALL _calcSet0BingingOffset(const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		bool SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, uint32_t set0BindingOffset, const ProgramDefine* defines, size_t numDefines, const ProgramIncludeHandler& includeHandler, const ProgramInputHandler& inputHandler, const ProgramTranspileHandler& transpileHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		bool SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const ProgramInputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		void SRK_CALL _parse(const SpvReflectShaderModule* data, const ProgramInputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		void SRK_CALL _parseParamLayout(const SpvReflectShaderModule* data, VkShaderStageFlags stageFlags, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		void SRK_CALL _parseParamLayout(const SpvReflectTypeDescription* data, struct SpvReflectBlockVariable* members, std::vector<ConstantBufferLayout::Variables>& vars);
		ConstantBuffer* _getConstantBuffer(const MyConstantBufferLayout& cbLayout, const IShaderParameterGetter& paramGetter);
		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);
	};
}