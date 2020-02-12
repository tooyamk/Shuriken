#pragma once

#include "aurora/renderers/commands/IRenderCommand.h"

namespace aurora::renderers::commands {
	template<typename T>
	class AE_TEMPLATE_DLL BaseRenderCommand : public IRenderCommand {
	protected:
		BaseRenderCommand() {
			_fn = &T::execute;
		}

		virtual ~BaseRenderCommand() {};
	};
}