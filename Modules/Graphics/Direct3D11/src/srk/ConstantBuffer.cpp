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
		return &_baseBuffer;
	}

	bool ConstantBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		using namespace srk::literals;

		auto quot = size >> 4;
		if (size & 0xF_uz) ++quot;
		return _baseBuffer.create(*_graphics.get<Graphics>(), quot << 4, bufferUsage, data, dataSize);
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

	size_t ConstantBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t ConstantBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	size_t ConstantBuffer::update(const void* data, size_t length, size_t offset) {
		return _baseBuffer.update(*_graphics.get<Graphics>(), data, length, offset);
	}

	size_t ConstantBuffer::copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		return _baseBuffer.copyFrom(*_graphics.get<Graphics>(), dstPos, src, srcRange);
	}

	bool ConstantBuffer::isSyncing() const {
		return false;
	}

	void ConstantBuffer::destroy() {
		_baseBuffer.releaseBuffer(*_graphics.get<Graphics>());
	}
}