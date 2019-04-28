#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
		_internalFormat(DXGI_FORMAT_UNKNOWN),
		_stride(0),
		_baseBuffer(D3D11_BIND_VERTEX_BUFFER) {
	}

	VertexBuffer::~VertexBuffer() {
		_baseBuffer.releaseBuffer(_graphics.get<Graphics>());
	}

	bool VertexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _baseBuffer.create(_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	Usage VertexBuffer::map(Usage mapUsage) {
		return _baseBuffer.map(_graphics.get<Graphics>(), mapUsage);
	}

	void VertexBuffer::unmap() {
		_baseBuffer.unmap(_graphics.get<Graphics>());
	}

	i32 VertexBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseBuffer.read(offset, dst, dstLen, readLen);
	}

	i32 VertexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(_graphics.get<Graphics>(), offset, data, length);
	}

	void VertexBuffer::flush() {
		_baseBuffer.flush();
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
		if (_baseBuffer.handle && _internalFormat != DXGI_FORMAT_UNKNOWN) {
			UINT offset = 0;
			_graphics.get<Graphics>()->getContext()->IASetVertexBuffers(slot, 1, (ID3D11Buffer**)&_baseBuffer.handle, &_stride, &offset);

			fmt = _internalFormat;

			return true;
		}
		return false;
	}
}