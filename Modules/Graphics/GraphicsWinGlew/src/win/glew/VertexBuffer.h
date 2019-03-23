#pragma once

#include "modules/graphics/VertexBuffer.h"

namespace aurora::modules::graphics::win::glew {
	class Graphics;

	class AE_MODULE_DLL VertexBuffer : public aurora::modules::graphics::VertexBuffer {
	public:
		VertexBuffer(Graphics* graphics);
		virtual ~VertexBuffer();

		virtual bool stroage(ui32 size, const void* data = nullptr) override;

	protected:
		Graphics* _graphics;
		ui32 _size;
		ui32 _handle;
		void* _mapData;
	};
}