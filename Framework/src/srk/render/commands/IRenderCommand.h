#pragma once

#include "srk/Framework.h"

namespace srk::render::commands {
	class SRK_FW_DLL IRenderCommand {
	public:
		virtual ~IRenderCommand();

		inline void SRK_CALL execute() {
			_fn(this);
		}

	protected:
		IRenderCommand();

		void(*_fn)(void*) = nullptr;
	};
}