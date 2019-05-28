#include "GraphicsUtils.h"

namespace aurora::modules::graphics {
	MultipleVertexBuffer::MultipleVertexBuffer(IGraphicsModule& graphics, ui8 max) : IVertexBuffer(graphics),
		_base(graphics, max) {
	}

	MultipleVertexBuffer::~MultipleVertexBuffer() {
	}

	const void* MultipleVertexBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleVertexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _base.create(size, bufferUsage, data, dataSize);
	}

	ui32 MultipleVertexBuffer::getSize() const {
		return _base.getSize();
	}

	Usage MultipleVertexBuffer::getUsage() const {
		return _base.getUsage();
	}

	Usage MultipleVertexBuffer::map(Usage expectMapUsage) {
		return _base.map(expectMapUsage);
	}

	void MultipleVertexBuffer::unmap() {
		_base.unmap();
	}

	ui32 MultipleVertexBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	ui32 MultipleVertexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _base.write(offset, data, length);
	}

	ui32 MultipleVertexBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _base.update(offset, data, length);
	}

	void MultipleVertexBuffer::getFormat(VertexSize* size, VertexType* type) const {
		_base.getFormat(size, type);
	}

	void MultipleVertexBuffer::setFormat(VertexSize size, VertexType type) {
		_base.setFormat(size, type);
	}

	void MultipleVertexBuffer::flush() {
	}

	bool MultipleVertexBuffer::isSyncing() const {
		return _base.isSyncing();
	}
}