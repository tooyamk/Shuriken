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
			VkVertexInputAttributeDescription* vertexInputAttribDescs, uint32_t& vertexInputAttribDescCount,
			const IShaderParameterGetter* shaderParamGetter);
		void SRK_CALL useEnd();

	protected:
		inline static constexpr std::string_view inputNamePrefix = std::string_view("in.var.");


		struct VertexLayout {
			uint32_t location;
		};


		class MyConstantBufferLayout : public ConstantBufferLayout {
		public:
			uint32_t sameId = 0;
		};


		struct TextureLayout {
			std::string name;
			uint32_t bindPoint = 0;
		};


		struct SamplerLayout {
			std::string name;
			uint32_t bindPoint = 0;
		};


		struct BindingLayout {
			VkDescriptorType type;
			uint32_t bindPoint;
			MyConstantBufferLayout data;

			inline void setName(const std::string_view& name) {
				data.name = name;
			}

			inline const std::string& getName() const {
				return data.name;
			}
		};


		struct SetLayout {
			uint32_t index;
			uint32_t constantBufferCount;
			std::vector<BindingLayout> bindings;
		};


		struct ParameterLayout {
			std::vector<SetLayout> sets;

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

		ParameterLayout _vsParamLayout;
		ParameterLayout _psParamLayout;

		std::vector<ConstantBuffer*> _usingSameConstBuffers;
		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		bool SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		void SRK_CALL _parse(const SpvReflectShaderModule* data, const InputHandler& inputHandler, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		void SRK_CALL _parseParamLayout(const SpvReflectShaderModule* data, ParameterLayout& layout, VkShaderStageFlags stageFlags, std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);
		void SRK_CALL _parseParamLayout(const SpvReflectTypeDescription* data, struct SpvReflectBlockVariable* members, std::vector<ConstantBufferLayout::Variables>& vars, VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding);
		void SRK_CALL _calcConstantLayoutSameBuffers(std::vector<std::vector<SetLayout>*>& setLayouts);
		ConstantBuffer* _getConstantBuffer(const MyConstantBufferLayout& cbLayout, const IShaderParameterGetter& paramGetter);
		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);
		void SRK_CALL _useParameters(const ParameterLayout& layout, const IShaderParameterGetter& paramGetter);
	};
}