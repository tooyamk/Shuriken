#pragma once

#include "BaseBuffer.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL IndexBuffer : public IIndexBuffer {
	public:
		IndexBuffer(Graphics& graphics);
		virtual ~IndexBuffer();

		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(size_t size, Usage bufferUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t SRK_CALL getSize() const override;
		virtual Usage SRK_CALL getUsage() const override;
		virtual Usage SRK_CALL map(Usage expectMapUsage) override;
		virtual void SRK_CALL unmap() override;
		virtual size_t SRK_CALL read(size_t offset, void* dst, size_t dstLen) override;
		virtual size_t SRK_CALL write(size_t offset, const void* data, size_t length) override;
		virtual size_t SRK_CALL update(size_t offset, const void* data, size_t length) override;
		virtual IndexType SRK_CALL getFormat() const override;
		virtual void SRK_CALL setFormat(IndexType type) override;
		//virtual void SRK_CALL flush() override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

		void SRK_CALL draw(uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) const;
		void SRK_CALL drawInstanced(uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) const;

	protected:
		IndexType _idxType;
		GLenum _internalType;
		uint32_t _numElements;
		BaseBuffer _baseBuffer;

		void _calcNumElements();
	};
}