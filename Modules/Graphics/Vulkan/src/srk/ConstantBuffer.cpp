#include "ConstantBuffer.h"

namespace srk::modules::graphics::vulkan {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : IConstantBuffer(graphics),
		_baseBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
		recordUpdateIds(nullptr) {
	}

	ConstantBuffer::~ConstantBuffer() {
		if (recordUpdateIds) delete[] recordUpdateIds;
	}

	bool ConstantBuffer::isCreated() const {
		return _baseBuffer.getBuffer();
	}

	const void* ConstantBuffer::getNative() const {
		return &_baseBuffer;
	}

	bool ConstantBuffer::create(size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, requiredUsage, preferredUsage, data, dataSize);
	}

	size_t ConstantBuffer::getSize() const {
		return _baseBuffer.getSize();
	}

	Usage ConstantBuffer::getUsage() const {
		return _baseBuffer.getUsage();
	}

	Usage ConstantBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void ConstantBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t ConstantBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t ConstantBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	size_t ConstantBuffer::update(const void* data, size_t length, size_t offset) {
		return _baseBuffer.update(data, length, offset);
	}

	size_t ConstantBuffer::copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		return _baseBuffer.copyFrom(*_graphics.get<Graphics>(), dstPos, src, srcRange);
	}

	bool ConstantBuffer::isSyncing() const {
		return false;
	}

	void ConstantBuffer::destroy() {
		_baseBuffer.destroy();
	}
}