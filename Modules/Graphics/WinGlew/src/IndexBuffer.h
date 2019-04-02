#pragma once

#include "Base.h"

namespace aurora::modules::graphics_win_glew {
	class Graphics;

	class AE_MODULE_DLL IndexBuffer : public IGraphicsIndexBuffer {
	public:
		IndexBuffer(Graphics& graphics);
		virtual ~IndexBuffer();

	protected:
		GLuint _handle;
	};
}