#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL BaseBuffer : public IVertexBuffer {
	public:
		BaseBuffer(Graphics& graphics);
		virtual ~BaseBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) override;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;
		virtual void AE_CALL use() override;

	protected:
		bool _dirty;
		ui32 _size;
		GLuint _handle;
		void* _mapData;

		GLsync _sync;

		void _delBuffer();
		void _waitServerSync();
		void _delSync();
	};
}