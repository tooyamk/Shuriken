#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL VertexBuffer : public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) override;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;
		virtual void AE_CALL use() override;

	protected:
		ID3D11Buffer* _handle;

		void _delBuffer();
		void _waitServerSync();
		void _delSync();
	};
}