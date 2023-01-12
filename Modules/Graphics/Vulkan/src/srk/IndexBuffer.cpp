#include "IndexBuffer.h"

namespace srk::modules::graphics::vulkan {
	IndexBuffer::IndexBuffer(Graphics& graphics) : IIndexBuffer(graphics),
		_type(IndexType::UNKNOWN),
		_baseBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
	}

	IndexBuffer::~IndexBuffer() {
	}

	bool IndexBuffer::isCreated() const {
		return _baseBuffer.getBuffer();
	}

	const void* IndexBuffer::getNative() const {
		return &_baseBuffer;
	}

	bool IndexBuffer::create(size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, requiredUsage, preferredUsage, data, dataSize);
	}

	size_t IndexBuffer::getSize() const {
		return _baseBuffer.getSize();
	}

	Usage IndexBuffer::getUsage() const {
		return _baseBuffer.getUsage();
	}

	Usage IndexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void IndexBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t IndexBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t IndexBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	size_t IndexBuffer::update(const void* data, size_t length, size_t offset) {
		return _baseBuffer.update(data, length, offset);
	}

	size_t IndexBuffer::copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		return _baseBuffer.copyFrom(*_graphics.get<Graphics>(), dstPos, src, srcRange);
	}

	bool IndexBuffer::isSyncing() const {
		return false;
	}

	void IndexBuffer::destroy() {
		_baseBuffer.destroy();
	}

	IndexType IndexBuffer::getFormat() const {
		return _type;
	}

	void IndexBuffer::setFormat(IndexType type) {
		_type = type;
	}
}