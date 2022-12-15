#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) override;
		virtual const ProgramInfo& getInfo() const override;
		virtual void SRK_CALL destroy() override;

		const std::vector<VkPipelineShaderStageCreateInfo>& getVkPipelineShaderStageCreateInfos() const {
			return _createInfos;
		}

		bool SRK_CALL use(const IVertexAttributeGetter* vertexAttributeGetter, VkVertexInputAttributeDescription* vertexInputAttribDescs, uint32_t& vertexInputAttribDescCount);

	protected:
		inline static constexpr std::string_view inputNamePrefix = std::string_view("in.var.");

		struct VertexLayout {
			uint32_t location;
		};

		std::vector<std::string> _entryPoints;
		std::vector<VkPipelineShaderStageCreateInfo> _createInfos;

		std::vector<VertexLayout> _vertexLayouts;

		bool _valid;
		ProgramInfo _info;

		bool SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler);
		void SRK_CALL _reflect(const void* data, const InputHandler& inputHandler);
	};
}