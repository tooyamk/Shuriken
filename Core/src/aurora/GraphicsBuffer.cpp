#include "GraphicsBuffer.h"
#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora {
	MultipleVertexBuffer::MultipleVertexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max) : modules::graphics::IVertexBuffer(graphics),
		_base(graphics, max) {
	}

	MultipleVertexBuffer::~MultipleVertexBuffer() {
	}

	bool MultipleVertexBuffer::isCreated() const {
		return _base.isCreated();
	}

	const void* MultipleVertexBuffer::getNative() const {
		return _base.getNative();
	}

	bool MultipleVertexBuffer::create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data, uint32_t dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target->setFormat(_format);
		return rst;
	}

	uint32_t MultipleVertexBuffer::getSize() const {
		return _base.getSize();
	}

	modules::graphics::Usage MultipleVertexBuffer::getUsage() const {
		return _base.getUsage();
	}

	modules::graphics::Usage MultipleVertexBuffer::map(modules::graphics::Usage expectMapUsage) {
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

	const modules::graphics::VertexFormat& MultipleVertexBuffer::getFormat() const {
		return _format;
	}

	void MultipleVertexBuffer::setFormat(const modules::graphics::VertexFormat& format) {
		if (!memEqual<sizeof(_format)>(&_format, &format)) {
			_format.size = format.size;
			_format.type = format.type;

			auto node = _base.getBegin();
			auto n = _base.getCount();
			while (n-- > 0) {
				node->target->setFormat(_format);
				node = node->next;
			};
		}
	}

	//void MultipleVertexBuffer::flush() {
	//}

	bool MultipleVertexBuffer::isSyncing() const {
		return _base.isSyncing();
	}

	void MultipleVertexBuffer::destroy() {
		_base.destroy();
	}


	MultipleIndexBuffer::MultipleIndexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max) : modules::graphics::IIndexBuffer(graphics),
		_idxType(modules::graphics::IndexType::UNKNOWN),
		_base(graphics, max) {
	}

	MultipleIndexBuffer::~MultipleIndexBuffer() {
	}

	bool MultipleIndexBuffer::isCreated() const {
		return _base.isCreated();
	}

	const void* MultipleIndexBuffer::getNative() const {
		return _base.getNative();
	}

	bool MultipleIndexBuffer::create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data, uint32_t dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target->setFormat(_idxType);
		return rst;
	}

	uint32_t MultipleIndexBuffer::getSize() const {
		return _base.getSize();
	}

	modules::graphics::Usage MultipleIndexBuffer::getUsage() const {
		return _base.getUsage();
	}

	modules::graphics::Usage MultipleIndexBuffer::map(modules::graphics::Usage expectMapUsage) {
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

	modules::graphics::IndexType MultipleIndexBuffer::getFormat() const {
		return _idxType;
	}

	void MultipleIndexBuffer::setFormat(modules::graphics::IndexType type) {
		if (_idxType != type) {
			_idxType = type;

			auto node = _base.getBegin();
			while (node) {
				node->target->setFormat(type);
				node = node->next;
			};
		}
	}

	//void MultipleIndexBuffer::flush() {
	//}

	bool MultipleIndexBuffer::isSyncing() const {
		return _base.isSyncing();
	}

	void MultipleIndexBuffer::destroy() {
		_base.destroy();
	}


	MultipleConstantBuffer::MultipleConstantBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max) : modules::graphics::IConstantBuffer(graphics),
		_base(graphics, max) {
	}

	MultipleConstantBuffer::~MultipleConstantBuffer() {
	}

	bool MultipleConstantBuffer::isCreated() const {
		return _base.isCreated();
	}

	const void* MultipleConstantBuffer::getNative() const {
		return _base.getNative();
	}

	bool MultipleConstantBuffer::create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data, uint32_t dataSize) {
		return _base.create(size, bufferUsage, data, dataSize);
	}

	uint32_t MultipleConstantBuffer::getSize() const {
		return _base.getSize();
	}

	modules::graphics::Usage MultipleConstantBuffer::getUsage() const {
		return _base.getUsage();
	}

	modules::graphics::Usage MultipleConstantBuffer::map(modules::graphics::Usage expectMapUsage) {
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

	void MultipleConstantBuffer::destroy() {
		_base.destroy();
	}


	VertexBufferCollection::~VertexBufferCollection() {
		clear();
	}

	modules::graphics::IVertexBuffer* VertexBufferCollection::get(const std::string& name) const {
		auto itr = _buffers.find(name);
		return itr == _buffers.end() ? nullptr : itr->second;
	}

	void VertexBufferCollection::set(const std::string& name, modules::graphics::IVertexBuffer* buffer) {
		if (auto itr = _buffers.find(name); buffer) {
			if (itr == _buffers.end()) {
				_buffers.emplace(name, buffer);
			} else if (itr->second != buffer) {
				itr->second = buffer;
			}
		} else if (itr != _buffers.end()) {
			_buffers.erase(itr);
		}
	}
}