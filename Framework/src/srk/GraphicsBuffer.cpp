#include "GraphicsBuffer.h"
#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk {
	MultipleVertexBuffer::MultipleVertexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max) : modules::graphics::IVertexBuffer(graphics),
		_stride(0),
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
		if (_base.getCurrent()) _base.getCurrent()->target->setStride(_stride);
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

	size_t MultipleVertexBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _base.read(dst, dstLen, offset);
	}

	size_t MultipleVertexBuffer::write(const void* data, size_t length, size_t offset) {
		return _base.write(data, length, offset);
	}

	size_t MultipleVertexBuffer::update(const void* data, size_t length, size_t offset) {
		return _base.update(data, length, offset);
	}

	size_t MultipleVertexBuffer::copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) {
		return _base.copyFrom(dstPos, src, srcRange);
	}

	bool MultipleVertexBuffer::isSyncing() const {
		return _base.isSyncing();
	}

	void MultipleVertexBuffer::destroy() {
		_base.destroy();
	}

	uint32_t MultipleVertexBuffer::getStride() const {
		return _stride;
	}

	void MultipleVertexBuffer::setStride(uint32_t stride) {
		_stride = stride;
		if (_stride != stride) {
			_stride = stride;

			auto node = _base.getBegin();
			auto n = _base.getCount();
			while (n-- > 0) {
				node->target->setStride(_stride);
				node = node->next;
			};
		}
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

	size_t MultipleIndexBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _base.read(dst, dstLen, offset);
	}

	size_t MultipleIndexBuffer::write(const void* data, size_t length, size_t offset) {
		return _base.write(data, length, offset);
	}

	size_t MultipleIndexBuffer::update(const void* data, size_t length, size_t offset) {
		return _base.update(data, length, offset);
	}

	size_t MultipleIndexBuffer::copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) {
		return _base.copyFrom(dstPos, src, srcRange);
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

	size_t MultipleConstantBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _base.read(dst, dstLen, offset);
	}

	size_t MultipleConstantBuffer::write(const void* data, size_t length, size_t offset) {
		return _base.write(data, length, offset);
	}

	size_t MultipleConstantBuffer::update(const void* data, size_t length, size_t offset) {
		return _base.update(data, length, offset);
	}

	size_t MultipleConstantBuffer::copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) {
		return _base.copyFrom(dstPos, src, srcRange);
	}

	bool MultipleConstantBuffer::isSyncing() const {
		return _base.isSyncing();
	}

	void MultipleConstantBuffer::destroy() {
		_base.destroy();
	}


	VertexAttributeCollection::~VertexAttributeCollection() {
		clear();
	}

	std::optional<modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>> VertexAttributeCollection::get(const QueryString& name) const {
		auto itr = _views.find(name);
		return itr == _views.end() ? std::nullopt : std::make_optional(itr->second);
	}

	void VertexAttributeCollection::set(const QueryString& name, const modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>& attrib) {
		if (auto itr = _views.find(name); itr == _views.end()) {
			_views.emplace(name, attrib);
		} else {
			itr->second = attrib;
		}
	}

	std::optional<modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>> VertexAttributeCollection::_remove(const QueryString& name) {
		if (auto itr = _views.find(name); itr != _views.end()) {
			auto opt = std::make_optional(std::move(itr->second));
			_views.erase(itr);
			return std::move(opt);
		}

		return std::nullopt;
	}
}