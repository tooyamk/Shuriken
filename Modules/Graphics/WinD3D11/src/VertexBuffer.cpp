#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	VertexBuffer::VertexBuffer(Graphics& graphics) : BaseBuffer(graphics, D3D11_BIND_VERTEX_BUFFER), IVertexBuffer(graphics),
		_internalFormat(DXGI_FORMAT_UNKNOWN),
		_stride(0) {
	}

	VertexBuffer::~VertexBuffer() {
	}

	bool VertexBuffer::stroage(ui32 size, const void* data) {
		return _stroage(size, data);
	}

	void VertexBuffer::write(ui32 offset, const void* data, ui32 length) {
		_write(offset, data, length);
	}

	void VertexBuffer::flush() {
		_flush();
	}

	void VertexBuffer::setFormat(VertexSize size, VertexType type) {
		switch (type) {
		case VertexType::I8:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R8_SINT;
				_stride = 1;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R8G8_SINT;
				_stride = 2;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R8G8B8A8_SINT;
				_stride = 4;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		case VertexType::UI8:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R8_UINT;
				_stride = 1;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R8G8_UINT;
				_stride = 2;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R8G8B8A8_UINT;
				_stride = 4;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		case VertexType::I16:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R16_SINT;
				_stride = 2;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R16G16_SINT;
				_stride = 4;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R16G16B16A16_SINT;
				_stride = 8;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		case VertexType::UI16:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R16_UINT;
				_stride = 2;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R16G16_UINT;
				_stride = 4;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R16G16B16A16_UINT;
				_stride = 8;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		case VertexType::I32:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R32_SINT;
				_stride = 4;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R32G32_SINT;
				_stride = 8;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_R32G32B32_SINT;
				_stride = 12;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R32G32B32A32_SINT;
				_stride = 16;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		case VertexType::UI32:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R32_UINT;
				_stride = 4;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R32G32_UINT;
				_stride = 8;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_R32G32B32_UINT;
				_stride = 12;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R32G32B32A32_UINT;
				_stride = 16;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		case VertexType::F32:
		{
			switch (size) {
			case VertexSize::ONE:
			{
				_internalFormat = DXGI_FORMAT_R32_FLOAT;
				_stride = 4;
				return;
			}
			case VertexSize::TWO:
			{
				_internalFormat = DXGI_FORMAT_R32G32_FLOAT;
				_stride = 8;
				return;
			}
			case VertexSize::THREE:
			{
				_internalFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				_stride = 12;
				return;
			}
			case VertexSize::FOUR:
			{
				_internalFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
				_stride = 16;
				return;
			}
			default:
			{
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				_stride = 0;
				return;
			}
			}
		}
		default:
		{
			_internalFormat = DXGI_FORMAT_UNKNOWN;
			_stride = 0;
			return;
		}
		}
	}

	bool VertexBuffer::use(UINT slot, DXGI_FORMAT& fmt) {
		if (_handle && _internalFormat != DXGI_FORMAT_UNKNOWN) {
			auto context = _grap->getContext();
			UINT offset = 0;
			context->IASetVertexBuffers(slot, 1, &_handle, &_stride, &offset);

			fmt = _internalFormat;

			return true;
		}
		return false;
	}
}