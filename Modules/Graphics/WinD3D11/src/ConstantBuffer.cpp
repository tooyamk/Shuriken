#include "ConstantBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
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

	bool ConstantBuffer::create(uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	uint32_t ConstantBuffer::getSize() const {
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

	uint32_t ConstantBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	uint32_t ConstantBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		return _baseBuffer.write(*_graphics.get<Graphics>(), offset, data, length);
	}

	uint32_t ConstantBuffer::update(uint32_t offset, const void* data, uint32_t length) {
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