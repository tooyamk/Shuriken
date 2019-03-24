#pragma once

#include "modules/graphics/VertexBuffer.h"
#include "GL/glew.h"

namespace aurora::modules::graphics::win::glew {
	class Graphics;

	class AE_MODULE_DLL VertexBuffer : public aurora::modules::graphics::VertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) override;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;
		virtual void AE_CALL use() override;

	protected:
		bool _dirty;
		ui32 _size;
		ui32 _handle;
		void* _mapData;

		GLsync _sync;

		void _delBuffer();
		void _waitServerSync();
		void _delSync();
	};
}