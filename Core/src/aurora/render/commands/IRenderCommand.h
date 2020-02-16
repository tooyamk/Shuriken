#pragma once

#include "aurora/Global.h"

namespace aurora::render::commands {
	class AE_DLL IRenderCommand {
	public:
		virtual ~IRenderCommand();

		inline void AE_CALL execute() {
			_fn(this);
		}

	protected:
		IRenderCommand();

		void(*_fn)(void*) = nullptr;
	};
}