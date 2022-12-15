#pragma once

#include "BaseBuffer.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL VertexBuffer : public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(size_t size, Usage bufferUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t SRK_CALL getSize() const override;
		virtual Usage SRK_CALL getUsage() const override;
		virtual Usage SRK_CALL map(Usage expectMapUsage) override;
		virtual void SRK_CALL unmap() override;
		virtual size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) override;
		virtual size_t SRK_CALL write(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL update(const void* data, size_t length, size_t offset) override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

		virtual uint32_t SRK_CALL getStride() const override;
		virtual void SRK_CALL setStride(uint32_t stride) override;

	protected:
		uint32_t _stride;

		BaseBuffer _baseBuffer;
	};
}