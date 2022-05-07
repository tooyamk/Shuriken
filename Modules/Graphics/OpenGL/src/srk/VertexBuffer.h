#pragma once

#include "BaseBuffer.h"

namespace srk::modules::graphics::gl {
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
		virtual size_t SRK_CALL read(size_t offset, void* dst, size_t dstLen) override;
		virtual size_t SRK_CALL write(size_t offset, const void* data, size_t length) override;
		virtual size_t SRK_CALL update(size_t offset, const void* data, size_t length) override;
		virtual const VertexFormat& SRK_CALL getFormat() const override;
		virtual void SRK_CALL setFormat(const VertexFormat& format) override;
		//virtual void SRK_CALL flush() override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

		bool SRK_CALL use(GLuint index, bool instanced);

	protected:
		VertexFormat _format;

		GLint _vertexSize;
		bool _validVertexFormat;
		GLenum _vertexType;
		BaseBuffer _baseBuffer;
	};
}