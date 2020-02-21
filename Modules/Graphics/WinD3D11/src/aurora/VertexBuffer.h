#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL VertexBuffer : public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual bool AE_CALL isCreated() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(uint32_t size, Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
		virtual const VertexFormat& AE_CALL getFormat() const override;
		virtual void AE_CALL setFormat(const VertexFormat& format) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;
		virtual void AE_CALL destroy() override;

		inline ID3D11Buffer* AE_CALL getInternalBuffer() const {
			return (ID3D11Buffer*)_baseBuffer.handle;
		}

		inline DXGI_FORMAT AE_CALL getInternalFormat() const {
			return _internalFormat;
		}

		inline UINT AE_CALL getStride() const {
			return _stride;
		}

	protected:
		VertexFormat _format;

		DXGI_FORMAT _internalFormat;
		UINT _stride;
		BaseBuffer _baseBuffer;
	};
}