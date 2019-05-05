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

	bool ConstantBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _baseBuffer.create(size, bufferUsage, data);
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

	i32 ConstantBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseBuffer.read(offset, dst, dstLen, readLen);
	}

	i32 ConstantBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(offset, data, length);
	}

	i32 ConstantBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.update(offset, data, length);
	}

	void ConstantBuffer::flush() {
		_baseBuffer.flush();
	}
}