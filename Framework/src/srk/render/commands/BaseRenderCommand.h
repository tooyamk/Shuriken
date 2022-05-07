#pragma once

#include "srk/render/commands/IRenderCommand.h"

namespace srk::render::commands {
	template<typename T>
	class BaseRenderCommand : public IRenderCommand {
	protected:
		BaseRenderCommand() {
			_fn = &T::execute;
		}

		virtual ~BaseRenderCommand() {};
	};
}