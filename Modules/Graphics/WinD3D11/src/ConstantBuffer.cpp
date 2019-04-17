#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : BaseBuffer(graphics, D3D11_BIND_CONSTANT_BUFFER), IConstantBuffer(graphics) {
	}

	ConstantBuffer::~ConstantBuffer() {
	}

	bool ConstantBuffer::stroage(ui32 size, const void* data) {
		return _stroage(size, data);
	}

	void ConstantBuffer::write(ui32 offset, const void* data, ui32 length) {
		_write(offset, data, length);
	}

	void ConstantBuffer::flush() {
		_flush();
	}
}