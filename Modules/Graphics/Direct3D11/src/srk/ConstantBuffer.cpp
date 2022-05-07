#include "ConstantBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	ConstantBuffer::ConstantBuffer(Graphics& graphics) : IConstantBuffer(graphics),
		_baseBuffer(D3D11_BIND_CONSTANT_BUFFER),
		recordUpdateIds(nullptr) {
	}

	ConstantBuffer::~ConstantBuffer() {
		if (recordUpdateIds) delete[] recordUpdateIds;
		destroy();
	}

	bool ConstantBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* ConstantBuffer::getNative() const {
		return this;
	}

	bool ConstantBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	size_t ConstantBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage ConstantBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage ConstantBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(*_graphics.get<Graphics>(), expectMapUsage);
	}

	void ConstantBuffer::unmap() {
		_baseBuffer.unmap(*_graphics.get<Graphics>());
	}

	size_t ConstantBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t ConstantBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(*_graphics.get<Graphics>(), offset, data, length);
	}

	size_t ConstantBuffer::update(size_t offset, const void* data, size_t length) {
		return _baseBuffer.update(*_graphics.get<Graphics>(), offset, data, length);
	}

	//void ConstantBuffer::flush() {
	//}

	bool ConstantBuffer::isSyncing() const {
		return false;
	}

	void ConstantBuffer::destroy() {
		_baseBuffer.releaseBuffer(*_graphics.get<Graphics>());
	}
}