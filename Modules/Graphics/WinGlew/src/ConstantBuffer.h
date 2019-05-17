#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL ConstantBuffer : public IConstantBuffer {
	public:
		ConstantBuffer(Graphics& graphics);
		virtual ~ConstantBuffer();

		ui32* recordUpdateIds;

		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) override;
		virtual ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;

		inline GLuint AE_CALL getInternalBuffer() const {
			return _baseBuffer.curHandle;
		}

	protected:
		BaseBuffer _baseBuffer;
	};
}