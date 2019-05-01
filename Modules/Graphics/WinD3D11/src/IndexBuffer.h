#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL IndexBuffer : public IIndexBuffer {
	public:
		IndexBuffer(Graphics& graphics);
		virtual ~IndexBuffer();

		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual i32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL setFormat(IndexType type) override;
		virtual void AE_CALL flush() override;

		void AE_CALL draw(ui32 count = 0xFFFFFFFFui32, ui32 offset = 0);

	protected:
		DXGI_FORMAT _indexType;
		ui32 _numElements;
		BaseBuffer _baseBuffer;

		void _calcNumElements();
	};
}