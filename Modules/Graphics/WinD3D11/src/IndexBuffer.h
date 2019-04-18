#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL IndexBuffer : private BaseBuffer, public IIndexBuffer {
	public:
		IndexBuffer(Graphics& graphics);
		virtual ~IndexBuffer();

		virtual bool AE_CALL stroage(ui32 size, ui32 bufferUsage, const void* data = nullptr) override;
		virtual bool AE_CALL map(ui32 mapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL setFormat(IndexType type) override;
		virtual void AE_CALL flush() override;

		void AE_CALL draw(ui32 count = 0xFFFFFFFFui32, ui32 offset = 0);

	protected:
		DXGI_FORMAT _indexType;
		ui32 _numElements;

		void _calcNumElements();
	};
}