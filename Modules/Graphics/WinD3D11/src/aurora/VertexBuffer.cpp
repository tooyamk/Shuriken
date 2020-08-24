#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
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

	bool VertexBuffer::create(uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	uint32_t VertexBuffer::getSize() const {
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

	uint32_t VertexBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	uint32_t VertexBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		return _baseBuffer.write(*_graphics.get<Graphics>(), offset, data, length);
	}

	uint32_t VertexBuffer::update(uint32_t offset, const void* data, uint32_t length) {
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