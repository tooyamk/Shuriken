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

	bool MultipleVertexBuffer::create(size_t size, modules::graphics::Usage bufferUsage, const void* data, size_t dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target->setFormat(_format);
		return rst;
	}

	size_t MultipleVertexBuffer::getSize() const {
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

	size_t MultipleVertexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	size_t MultipleVertexBuffer::write(size_t offset, const void* data, size_t length) {
		return _base.write(offset, data, length);
	}

	size_t MultipleVertexBuffer::update(size_t offset, const void* data, size_t length) {
		return _base.update(offset, data, length);
	}

	const modules::graphics::VertexFormat& MultipleVertexBuffer::getFormat() const {
		return _format;
	}

	void MultipleVertexBuffer::setFormat(const modules::graphics::VertexFormat& format) {
		if (_format != format) {
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

	bool MultipleIndexBuffer::create(size_t size, modules::graphics::Usage bufferUsage, const void* data, size_t dataSize) {
		auto rst = _base.create(size, bufferUsage, data, dataSize);
		if (_base.getCurrent()) _base.getCurrent()->target->setFormat(_idxType);
		return rst;
	}

	size_t MultipleIndexBuffer::getSize() const {
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

	size_t MultipleIndexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	size_t MultipleIndexBuffer::write(size_t offset, const void* data, size_t length) {
		return _base.write(offset, data, length);
	}

	size_t MultipleIndexBuffer::update(size_t offset, const void* data, size_t length) {
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

	bool MultipleConstantBuffer::create(size_t size, modules::graphics::Usage bufferUsage, const void* data, size_t dataSize) {
		return _base.create(size, bufferUsage, data, dataSize);
	}

	size_t MultipleConstantBuffer::getSize() const {
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

	size_t MultipleConstantBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _base.read(offset, dst, dstLen);
	}

	size_t MultipleConstantBuffer::write(size_t offset, const void* data, size_t length) {
		return _base.write(offset, data, length);
	}

	size_t MultipleConstantBuffer::update(size_t offset, const void* data, size_t length) {
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

	RefPtr<modules::graphics::IVertexBuffer> VertexBufferCollection::get(const query_string& name) const {
		auto itr = _buffers.find(name);
		return itr == _buffers.end() ? nullptr : itr->second;
	}

	void VertexBufferCollection::set(const query_string& name, modules::graphics::IVertexBuffer* buffer) {
		if (buffer) {
			if (auto itr = _buffers.find(name); itr == _buffers.end()) {
				_buffers.emplace(name, buffer);
			} else {
				itr->second = buffer;
			}
		} else {
			remove(name);
		}
	}

	modules::graphics::IVertexBuffer* VertexBufferCollection::_remove(const query_string& name) {
		if (auto itr = _buffers.find(name); itr == _buffers.end()) {
			auto val = itr->second;
			_buffers.erase(itr);
			return val;
		}

		return nullptr;
	}
}