#pragma once

#include "srk/ProgramSource.h"
#include "srk/ShaderDefine.h"
#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk::modules::graphics {
	class IGraphicsModule;
	class IProgram;
}

namespace srk {
	class ShaderDefine;
	class IShaderDefineGetter;


	class SRK_FW_DLL Shader : public Ref {
	public:
		using IncludeHandler = std::function<ByteArray(const Shader&, ProgramStage, const std::string_view&)>;
		using InputHandler = std::function<modules::graphics::IProgram::InputDescriptor(const Shader&, const std::string_view&)>;

		Shader();

		void SRK_CALL set(modules::graphics::IGraphicsModule* graphics, ProgramSource* vs, ProgramSource* ps, 
			const ShaderDefine* staticDefines, size_t numStaticDefines, const std::string_view* dynamicDefines, size_t numDynamicDefines,
			const IncludeHandler& includeHandler, const InputHandler& inputHandler);
		IntrusivePtr<modules::graphics::IProgram> SRK_CALL select(const IShaderDefineGetter* getter);

		void SRK_CALL unset();

		void SRK_CALL setVariant(ProgramSource* vs, ProgramSource* ps, const IShaderDefineGetter* getter);

	protected:
		struct Variant {
			IntrusivePtr<ProgramSource> vs;
			IntrusivePtr<ProgramSource> ps;
		};


		IntrusivePtr<modules::graphics::IGraphicsModule> _graphics;
		IntrusivePtr<ProgramSource> _vs;
		IntrusivePtr<ProgramSource> _ps;
		IncludeHandler _includeHhandler;
		InputHandler _inputHandler;

		std::unordered_map<uint64_t, Variant> _variants;

		std::vector<std::string> _staticDefines;
		std::vector<std::string> _dynamicDefines;
		std::vector<ShaderDefine> _defines;

		std::unordered_map<uint64_t, IntrusivePtr<modules::graphics::IProgram>> _programs;
	};
}