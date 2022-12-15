#include "VertexBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
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

	size_t VertexBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t VertexBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	size_t VertexBuffer::update(const void* data, size_t length, size_t offset) {
		return _baseBuffer.update(*_graphics.get<Graphics>(), data, length, offset);
	}

	//void VertexBuffer::flush() {
	//}

	bool VertexBuffer::isSyncing() const {
		return false;
	}

	void VertexBuffer::destroy() {
		_baseBuffer.releaseBuffer(*_graphics.get<Graphics>());
	}

	uint32_t VertexBuffer::getStride() const {
		return _stride;
	}

	void VertexBuffer::setStride(uint32_t stride) {
		_stride = stride;
	}
}