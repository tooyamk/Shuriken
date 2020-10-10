#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::d3d11 {
	class AE_MODULE_DLL IndexBuffer : public IIndexBuffer {
	public:
		IndexBuffer(Graphics& graphics);
		virtual ~IndexBuffer();

		virtual bool AE_CALL isCreated() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(size_t size, Usage bufferUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual size_t AE_CALL read(size_t offset, void* dst, size_t dstLen) override;
		virtual size_t AE_CALL write(size_t offset, const void* data, size_t length) override;
		virtual size_t AE_CALL update(size_t offset, const void* data, size_t length) override;
		virtual IndexType AE_CALL getFormat() const override;
		virtual void AE_CALL setFormat(IndexType type) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;
		virtual void AE_CALL destroy() override;

		inline ID3D11Buffer* AE_CALL getInternalBuffer() const {
			return (ID3D11Buffer*)_baseBuffer.handle;
		}

		inline DXGI_FORMAT AE_CALL getInternalFormat() const {
			return _internalFormat;
		}

		inline uint32_t AE_CALL getNumElements() const {
			return _numElements;
		}

		void AE_CALL draw (uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0);

	protected:
		IndexType _idxType;
		DXGI_FORMAT _internalFormat;
		uint32_t _numElements;
		BaseBuffer _baseBuffer;

		void _calcNumElements();
	};
}