#include "GraphicsUtils.h"

namespace aurora::modules::graphics {
	MultipleVertexBuffer::MultipleVertexBuffer(IGraphicsModule& graphics, uint8_t max) : IVertexBuffer(graphics),
		_vertSize(VertexSize::UNKNOWN),
		_vertType(VertexType::UNKNOWN),
		_base(graphics, max) {
	}

	MultipleVertexBuffer::~MultipleVertexBuffer() {
	}

	const void* MultipleVertexBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleVertexBuffer::create (uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target.get()->setFormat(_vertSize, _vertType);
		return rst;
	}

	uint32_t MultipleVertexBuffer::getSize() const {
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

	uint32_t MultipleVertexBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	uint32_t MultipleVertexBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		return _base.write(offset, data, length);
	}

	uint32_t MultipleVertexBuffer::update(uint32_t offset, const void* data, uint32_t length) {
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


	MultipleIndexBuffer::MultipleIndexBuffer(IGraphicsModule& graphics, uint8_t max) : IIndexBuffer(graphics),
		_idxType(IndexType::UNKNOWN),
		_base(graphics, max) {
	}

	MultipleIndexBuffer::~MultipleIndexBuffer() {
	}

	const void* MultipleIndexBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleIndexBuffer::create (uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target.get()->setFormat(_idxType);
		return rst;
	}

	uint32_t MultipleIndexBuffer::getSize() const {
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

	uint32_t MultipleIndexBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	uint32_t MultipleIndexBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		return _base.write(offset, data, length);
	}

	uint32_t MultipleIndexBuffer::update(uint32_t offset, const void* data, uint32_t length) {
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


	MultipleConstantBuffer::MultipleConstantBuffer(IGraphicsModule& graphics, uint8_t max) : IConstantBuffer(graphics),
		_base(graphics, max) {
	}

	MultipleConstantBuffer::~MultipleConstantBuffer() {
	}

	const void* MultipleConstantBuffer::getNativeBuffer() const {
		return _base.getNativeBuffer();
	}

	bool MultipleConstantBuffer::create (uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		return _base.create(size, bufferUsage, data, dataSize);
	}

	uint32_t MultipleConstantBuffer::getSize() const {
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

	uint32_t MultipleConstantBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	uint32_t MultipleConstantBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		return _base.write(offset, data, length);
	}

	uint32_t MultipleConstantBuffer::update(uint32_t offset, const void* data, uint32_t length) {
		return _base.update(offset, data, length);
	}

	//void MultipleConstantBuffer::flush() {
	//}

	bool MultipleConstantBuffer::isSyncing() const {
		return _base.isSyncing();
	}
}