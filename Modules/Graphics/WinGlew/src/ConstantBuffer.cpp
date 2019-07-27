#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : IConstantBuffer(graphics),
		_baseBuffer(GL_UNIFORM_BUFFER),
		recordUpdateIds(nullptr) {
	}

	ConstantBuffer::~ConstantBuffer() {
		if (recordUpdateIds) delete[] recordUpdateIds;
	}

	const void* ConstantBuffer::getNativeBuffer() const {
		return this;
	}

	bool ConstantBuffer::create (uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
	}

	uint32_t ConstantBuffer::getSize() const {
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

	uint32_t ConstantBuffer::read (uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	uint32_t ConstantBuffer::write (uint32_t offset, const void* data, uint32_t length) {
		return _baseBuffer.write(offset, data, length);
	}

	uint32_t ConstantBuffer::update (uint32_t offset, const void* data, uint32_t length) {
		return _baseBuffer.update(offset, data, length);
	}

	//void ConstantBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool ConstantBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}
}