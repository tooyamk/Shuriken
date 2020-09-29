#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::gl {
	class AE_MODULE_DLL PixelBuffer : public IPixelBuffer {
	public:
		PixelBuffer(Graphics& graphics);
		virtual ~PixelBuffer();

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
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;
		virtual void AE_CALL destroy() override;

		inline GLenum AE_CALL getInternalType() const {
			return _baseBuffer.bufferType;
		}

		inline GLuint AE_CALL getInternalBuffer() const {
			return _baseBuffer.handle;
		}

		inline BaseBuffer* AE_CALL getBaseBuffer() {
			return &_baseBuffer;
		}

	protected:
		BaseBuffer _baseBuffer;
	};
}