#pragma once

#include "BaseBuffer.h"

namespace srk::modules::graphics::d3d11 {
	class SRK_MODULE_DLL IndexBuffer : public IIndexBuffer {
	public:
		IndexBuffer(Graphics& graphics);
		virtual ~IndexBuffer();

		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(size_t size, Usage requiredUsage, Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t SRK_CALL getSize() const override;
		virtual Usage SRK_CALL getUsage() const override;
		virtual Usage SRK_CALL map(Usage expectMapUsage) override;
		virtual void SRK_CALL unmap() override;
		virtual size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) override;
		virtual size_t SRK_CALL write(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL update(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

		virtual IndexType SRK_CALL getFormat() const override;
		virtual void SRK_CALL setFormat(IndexType type) override;

		inline DXGI_FORMAT SRK_CALL getInternalFormat() const {
			return _internalFormat;
		}

		inline uint32_t SRK_CALL getNumElements() const {
			return _numElements;
		}

		void SRK_CALL draw(uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0);

	protected:
		IndexType _idxType;
		DXGI_FORMAT _internalFormat;
		uint32_t _numElements;
		BaseBuffer _baseBuffer;

		void _calcNumElements();
	};
}