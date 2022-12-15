#include "PixelBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	PixelBuffer::PixelBuffer(Graphics& graphics) : IPixelBuffer(graphics),
		_baseBuffer(0) {
	}

	PixelBuffer::~PixelBuffer() {
	}

	bool PixelBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* PixelBuffer::getNative() const {
		return this;
	}

	bool PixelBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		destroy();

		bool read = (bufferUsage & Usage::MAP_READ) == Usage::MAP_READ;
		bool write = ((bufferUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) || ((bufferUsage & Usage::UPDATE) == Usage::UPDATE);

		if (read && write) {
			_graphics.get<Graphics>()->error("OpenGL PixelBuffer::create error, could not enable Usage::MAP_READ and (Usage::MAP_WRITE or Usage::UPDATE) at same time");
			return false;
		} else {
			if (read) {
				_baseBuffer.bufferType = GL_PIXEL_PACK_BUFFER;
			} else if (write) {
				_baseBuffer.bufferType = GL_PIXEL_UNPACK_BUFFER;
			} else {
				return true;
			}

			return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, GL_DYNAMIC_READ);
		}
	}

	size_t PixelBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage PixelBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage PixelBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage, GL_READ_WRITE);
	}

	void PixelBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t PixelBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t PixelBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	size_t PixelBuffer::update(const void* data, size_t length, size_t offset) {
		return _baseBuffer.update(data, length, offset);
	}

	//void PixelBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool PixelBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void PixelBuffer::destroy() {
		_baseBuffer.releaseBuffer();
	}
}