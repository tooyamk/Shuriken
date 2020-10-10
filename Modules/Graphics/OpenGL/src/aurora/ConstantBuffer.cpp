#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::gl {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : IConstantBuffer(graphics),
		_baseBuffer(GL_UNIFORM_BUFFER),
		recordUpdateIds(nullptr) {
	}

	ConstantBuffer::~ConstantBuffer() {
		if (recordUpdateIds) delete[] recordUpdateIds;
	}

	bool ConstantBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* ConstantBuffer::getNative() const {
		return this;
	}

	bool ConstantBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
	}

	size_t ConstantBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage ConstantBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage ConstantBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void ConstantBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t ConstantBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t ConstantBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(offset, data, length);
	}

	size_t ConstantBuffer::update(size_t offset, const void* data, size_t length) {
		return _baseBuffer.update(offset, data, length);
	}

	//void ConstantBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool ConstantBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void ConstantBuffer::destroy() {
		_baseBuffer.releaseBuffer();
	}
}