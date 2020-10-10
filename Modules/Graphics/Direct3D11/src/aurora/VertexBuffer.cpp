#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::d3d11 {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
		_internalFormat(DXGI_FORMAT_UNKNOWN),
		_stride(0),
		_baseBuffer(D3D11_BIND_VERTEX_BUFFER) {
	}

	VertexBuffer::~VertexBuffer() {
		destroy();
	}

	bool VertexBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* VertexBuffer::getNative() const {
		return this;
	}

	bool VertexBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	size_t VertexBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage VertexBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage VertexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(*_graphics.get<Graphics>(), expectMapUsage);
	}

	void VertexBuffer::unmap() {
		_baseBuffer.unmap(*_graphics.get<Graphics>());
	}

	size_t VertexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t VertexBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(*_graphics.get<Graphics>(), offset, data, length);
	}

	size_t VertexBuffer::update(size_t offset, const void* data, size_t length) {
		return _baseBuffer.update(*_graphics.get<Graphics>(), offset, data, length);
	}

	//void VertexBuffer::flush() {
	//}

	bool VertexBuffer::isSyncing() const {
		return false;
	}

	void VertexBuffer::destroy() {
		_baseBuffer.releaseBuffer(*_graphics.get<Graphics>());
	}

	const VertexFormat& VertexBuffer::getFormat() const {
		return _format;
	}

	void VertexBuffer::setFormat(const VertexFormat& format) {
		if (_format != format) {
			_format.size = format.size;
			_format.type = format.type;

			switch (_format.type) {
			case VertexType::I8:
			{
				switch (_format.size) {
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
				switch (_format.size) {
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
				switch (_format.size) {
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
				switch (_format.size) {
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
				switch (_format.size) {
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
				switch (_format.size) {
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
				switch (_format.size) {
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
	}
}