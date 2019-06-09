#include "GraphicsUtils.h"

namespace aurora::modules::graphics {
	MultipleVertexBuffer::MultipleVertexBuffer(IGraphicsModule& graphics, ui8 max) : IVertexBuffer(graphics),
		_vertSize(VertexSize::UNKNOWN),
		_vertType(VertexType::UNKNOWN),
		_base(graphics, max) {
	}

	MultipleVertexBuffer::~MultipleVertexBuffer() {
	}

	const void* MultipleVertexBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleVertexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target.get()->setFormat(_vertSize, _vertType);
		return rst;
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
		if (size) *size = _vertSize;
		if (type) *type = _vertType;
	}

	void MultipleVertexBuffer::setFormat(VertexSize size, VertexType type) {
		if (_vertSize != size || _vertType != type) {
			_vertSize = size;
			_vertType = type;

			auto node = _base.getBegin();
			auto n = _base.getCount();
			while (n-- > 0) {
				node->target.get()->setFormat(size, type);
				node = node->next;
			};
		}
	}

	//void MultipleVertexBuffer::flush() {
	//}

	bool MultipleVertexBuffer::isSyncing() const {
		return _base.isSyncing();
	}


	MultipleIndexBuffer::MultipleIndexBuffer(IGraphicsModule& graphics, ui8 max) : IIndexBuffer(graphics),
		_idxType(IndexType::UNKNOWN),
		_base(graphics, max) {
	}

	MultipleIndexBuffer::~MultipleIndexBuffer() {
	}

	const void* MultipleIndexBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleIndexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target.get()->setFormat(_idxType);
		return rst;
	}

	ui32 MultipleIndexBuffer::getSize() const {
		return _base.getSize();
	}

	Usage MultipleIndexBuffer::getUsage() const {
		return _base.getUsage();
	}

	Usage MultipleIndexBuffer::map(Usage expectMapUsage) {
		return _base.map(expectMapUsage);
	}

	void MultipleIndexBuffer::unmap() {
		_base.unmap();
	}

	ui32 MultipleIndexBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	ui32 MultipleIndexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _base.write(offset, data, length);
	}

	ui32 MultipleIndexBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _base.update(offset, data, length);
	}

	IndexType MultipleIndexBuffer::getFormat() const {
		return _idxType;
	}

	void MultipleIndexBuffer::setFormat(IndexType type) {
		if (_idxType != type) {
			_idxType = type;

			auto node = _base.getBegin();
			while (node) {
				node->target.get()->setFormat(type);
				node = node->next;
			};
		}
	}

	//void MultipleIndexBuffer::flush() {
	//}

	bool MultipleIndexBuffer::isSyncing() const {
		return _base.isSyncing();
	}


	MultipleConstantBuffer::MultipleConstantBuffer(IGraphicsModule& graphics, ui8 max) : IConstantBuffer(graphics),
		_base(graphics, max) {
	}

	MultipleConstantBuffer::~MultipleConstantBuffer() {
	}

	const void* MultipleConstantBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleConstantBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _base.create(size, bufferUsage, data, dataSize);
	}

	ui32 MultipleConstantBuffer::getSize() const {
		return _base.getSize();
	}

	Usage MultipleConstantBuffer::getUsage() const {
		return _base.getUsage();
	}

	Usage MultipleConstantBuffer::map(Usage expectMapUsage) {
		return _base.map(expectMapUsage);
	}

	void MultipleConstantBuffer::unmap() {
		_base.unmap();
	}

	ui32 MultipleConstantBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	ui32 MultipleConstantBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _base.write(offset, data, length);
	}

	ui32 MultipleConstantBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _base.update(offset, data, length);
	}

	//void MultipleConstantBuffer::flush() {
	//}

	bool MultipleConstantBuffer::isSyncing() const {
		return _base.isSyncing();
	}
}