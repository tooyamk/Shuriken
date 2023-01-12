#pragma once

#include "BaseBuffer.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL VertexBuffer : public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

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

		virtual size_t SRK_CALL getStride() const override;
		virtual void SRK_CALL setStride(size_t stride) override;

	protected:
		size_t _stride;
		BaseBuffer _baseBuffer;
	};
}