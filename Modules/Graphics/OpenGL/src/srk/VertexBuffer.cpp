#include "VertexBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
		_stride(0),
		_baseBuffer(GL_ARRAY_BUFFER) {
	}

	VertexBuffer::~VertexBuffer() {
	}

	bool VertexBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* VertexBuffer::getNative() const {
		return this;
	}

	bool VertexBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
	}

	size_t VertexBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage VertexBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage VertexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void VertexBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t VertexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t VertexBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(offset, data, length);
	}

	size_t VertexBuffer::update(size_t offset, const void* data, size_t length) {
		return _baseBuffer.update(offset, data, length);
	}

	//void VertexBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool VertexBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void VertexBuffer::destroy() {
		_baseBuffer.releaseBuffer();
	}

	uint32_t VertexBuffer::getStride() const {
		return _stride;
	}

	void VertexBuffer::setStride(uint32_t stride) {
		_stride = stride;
	}
}