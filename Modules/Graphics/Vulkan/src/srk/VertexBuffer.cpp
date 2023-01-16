#include "VertexBuffer.h"

namespace srk::modules::graphics::vulkan {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
		_stride(0),
		_baseBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
	}

	VertexBuffer::~VertexBuffer() {
	}

	bool VertexBuffer::isCreated() const {
		return _baseBuffer.getVkBuffer();
	}

	const void* VertexBuffer::getNative() const {
		return &_baseBuffer;
	}

	bool VertexBuffer::create(size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, requiredUsage, preferredUsage, data, dataSize);
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
		return _baseBuffer.update(data, length, offset);
	}

	size_t VertexBuffer::copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		return _baseBuffer.copyFrom(*_graphics.get<Graphics>(), dstPos, src, srcRange);
	}

	bool VertexBuffer::isSyncing() const {
		return false;
	}

	void VertexBuffer::destroy() {
		_baseBuffer.destroy();
	}

	size_t VertexBuffer::getStride() const {
		return _stride;
	}

	void VertexBuffer::setStride(size_t stride) {
		_stride = stride;
	}
}