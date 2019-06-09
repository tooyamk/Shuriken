#pragma once

#include "BaseBuffer.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL VertexBuffer : public IVertexBuffer {
	public:
		VertexBuffer(Graphics& graphics);
		virtual ~VertexBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual ui32 AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) override;
		virtual ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL getFormat(VertexSize* size, VertexType* type) const override;
		virtual void AE_CALL setFormat(VertexSize size, VertexType type) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;

		bool AE_CALL use(GLuint index);

	protected:
		VertexSize _vertSize;
		VertexType _vertType;

		GLint _vertexSize;
		bool _validVertexFormat;
		GLenum _vertexType;
		BaseBuffer _baseBuffer;
	};
}