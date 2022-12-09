#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class Graphics;

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

	protected:
		inline static constexpr uint32_t InputNameSkipLength = 7;

		std::vector<std::string> _entryPoints;
		std::vector<VkPipelineShaderStageCreateInfo> _createInfos;

		ProgramInfo _info;

		bool SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler);
		void SRK_CALL _reflect(const void* data);
	};
}