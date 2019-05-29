#include "PixelBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	PixelBuffer::PixelBuffer(Graphics& graphics) : IPixelBuffer(graphics),
		_baseBuffer(0) {
	}

	PixelBuffer::~PixelBuffer() {
	}

	const void* PixelBuffer::getNativeBuffer() const {
		return this;
	}

	bool PixelBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		_baseBuffer.releaseBuffer();

		bool read = (bufferUsage & Usage::MAP_READ) == Usage::MAP_READ;
		bool write = ((bufferUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) || ((bufferUsage & Usage::UPDATE) == Usage::UPDATE);

		if (read && write) {
			println("PixelBuffer.create error");
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

	ui32 PixelBuffer::getSize() const {
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

	ui32 PixelBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	ui32 PixelBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(offset, data, length);
	}

	ui32 PixelBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.update(offset, data, length);
	}

	void PixelBuffer::flush() {
		_baseBuffer.flush();
	}

	bool PixelBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}
}