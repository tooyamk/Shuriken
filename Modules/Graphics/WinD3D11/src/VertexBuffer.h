#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL VertexBuffer : private BaseBuffer, public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) override;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL setFormat(VertexSize size, VertexType type) override;
		virtual void AE_CALL flush() override;

		bool AE_CALL use(UINT slot, DXGI_FORMAT& fmt);

	protected:
		DXGI_FORMAT _internalFormat;
		UINT _stride;
	};
}