#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : BaseBuffer(graphics, D3D11_BIND_CONSTANT_BUFFER), IConstantBuffer(graphics) {
	}

	ConstantBuffer::~ConstantBuffer() {
	}

	bool ConstantBuffer::stroage(ui32 size, ui32 bufferUsage, const void* data) {
		return _stroage(size, bufferUsage, data);
	}

	bool ConstantBuffer::map(ui32 mapUsage) {
		return _map(mapUsage);
	}

	void ConstantBuffer::unmap() {
		_unmap();
	}

	i32 ConstantBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _read(offset, dst, dstLen, readLen);
	}

	i32 ConstantBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _write(offset, data, length);
	}

	void ConstantBuffer::flush() {
		_flush();
	}
}