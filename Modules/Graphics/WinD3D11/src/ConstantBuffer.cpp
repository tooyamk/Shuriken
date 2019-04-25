#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : IConstantBuffer(graphics),
		_baseBuffer(D3D11_BIND_CONSTANT_BUFFER),
		recordUpdateIds(nullptr) {
	}

	ConstantBuffer::~ConstantBuffer() {
		if (recordUpdateIds) delete[] recordUpdateIds;
		_baseBuffer.releaseRes((Graphics*)_graphics);
	}

	bool ConstantBuffer::allocate(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _baseBuffer.allocate((Graphics*)_graphics, size, bufferUsage, data, dataSize);
	}

	Usage ConstantBuffer::map(Usage mapUsage) {
		return _baseBuffer.map((Graphics*)_graphics, mapUsage);
	}

	void ConstantBuffer::unmap() {
		_baseBuffer.unmap((Graphics*)_graphics);
	}

	i32 ConstantBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseBuffer.read(offset, dst, dstLen, readLen);
	}

	i32 ConstantBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write((Graphics*)_graphics, offset, data, length);
	}

	void ConstantBuffer::flush() {
		_baseBuffer.flush();
	}
}