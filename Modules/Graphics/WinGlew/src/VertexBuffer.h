#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL VertexBuffer : public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual i32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL setFormat(VertexSize size, VertexType type) override;
		virtual void AE_CALL flush() override;

		bool AE_CALL use(GLuint index);

	protected:
		GLint _vertexSize;
		bool _validVertexFormat;
		GLenum _vertexType;
		BaseBuffer _baseBuffer;
	};
}