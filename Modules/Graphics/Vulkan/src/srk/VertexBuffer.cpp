#include "VertexBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
		_stride(0),
		_baseBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
	}

	VertexBuffer::~VertexBuffer() {
		destroy();
	}

	bool VertexBuffer::isCreated() const {
		return _baseBuffer.getBuffer();
	}

	const void* VertexBuffer::getNative() const {
		return this;
	}

	bool VertexBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	size_t VertexBuffer::getSize() const {
		return _baseBuffer.getSize();
	}

	Usage VertexBuffer::getUsage() const {
		return _baseBuffer.getUsage();
	}

	Usage VertexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void VertexBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t VertexBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t VertexBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	size_t VertexBuffer::update(const void* data, size_t length, size_t offset) {
		return -1;
	}

	bool VertexBuffer::isSyncing() const {
		return false;
	}

	void VertexBuffer::destroy() {
		_baseBuffer.destroy();
	}

	uint32_t VertexBuffer::getStride() const {
		return _stride;
	}

	void VertexBuffer::setStride(uint32_t stride) {
		_stride = stride;
	}
}