#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : IConstantBuffer(graphics),
		_baseBuffer(D3D11_BIND_CONSTANT_BUFFER),
		recordUpdateIds(nullptr) {
	}

	ConstantBuffer::~ConstantBuffer() {
		if (recordUpdateIds) delete[] recordUpdateIds;
		_baseBuffer.releaseBuffer(_graphics.get<Graphics>());
	}

	bool ConstantBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _baseBuffer.create(_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	Usage ConstantBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(_graphics.get<Graphics>(), expectMapUsage);
	}

	void ConstantBuffer::unmap() {
		_baseBuffer.unmap(_graphics.get<Graphics>());
	}

	i32 ConstantBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseBuffer.read(offset, dst, dstLen, readLen);
	}

	i32 ConstantBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(_graphics.get<Graphics>(), offset, data, length);
	}

	i32 ConstantBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.update(_graphics.get<Graphics>(), offset, data, length);
	}

	void ConstantBuffer::flush() {
		_baseBuffer.flush();
	}
}