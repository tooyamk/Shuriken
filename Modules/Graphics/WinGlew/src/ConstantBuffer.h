#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL ConstantBuffer : public IConstantBuffer {
	public:
		ConstantBuffer(Graphics& graphics);
		virtual ~ConstantBuffer();

		uint32_t* recordUpdateIds;

		virtual const void* AE_CALL getNativeBuffer() const override;
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

		inline GLuint AE_CALL getInternalBuffer() const {
			return _baseBuffer.handle;
		}

	protected:
		BaseBuffer _baseBuffer;
	};
}