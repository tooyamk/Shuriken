#include "PixelBuffer.h"
#include "BaseTexture.h"

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
		return &_baseBuffer;
	}

	bool PixelBuffer::create(size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		destroy();

		auto allUsage = requiredUsage | preferredUsage;
		auto read = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
		auto write = (allUsage & Usage::MAP_WRITE_UPDATE) != Usage::NONE;

		if (read && write) {
			auto requiredRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
			auto requiredWrite = (allUsage & Usage::MAP_WRITE_UPDATE) != Usage::NONE;
			if (requiredRead) {
				if (requiredWrite) {
					_graphics.get<Graphics>()->error("OpenGL PixelBuffer::create error, could not enable Usage::MAP_READ and (Usage::MAP_WRITE or Usage::UPDATE) at same time");
					return false;
				} else {
					write = false;
					preferredUsage &= ~Usage::MAP_WRITE_UPDATE;
				}
			} else if (requiredWrite) {
				read = false;
				preferredUsage &= ~Usage::MAP_READ;
			} else {
				write = false;
				preferredUsage &= ~Usage::MAP_WRITE_UPDATE;
			}
		}

		if (read) {
			_baseBuffer.bufferType = GL_PIXEL_PACK_BUFFER;
		} else if (write) {
			_baseBuffer.bufferType = GL_PIXEL_UNPACK_BUFFER;
		} else {
			return true;
		}

		return _baseBuffer.create(*_graphics.get<Graphics>(), size, requiredUsage, preferredUsage, data, GL_DYNAMIC_READ);
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

	size_t PixelBuffer::copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		return _baseBuffer.copyFrom(*_graphics.get<Graphics>(), dstPos, src, srcRange);
	}

	bool PixelBuffer::copyFrom(uint32_t mipSlice, const ITextureResource* src) {
		if (!src || _graphics != src->getGraphics()) return false;

		auto native = (BaseTexture*)src->getNative();
		if (!native) return false;

		if (!_baseBuffer.handle || _baseBuffer.bufferType != GL_PIXEL_PACK_BUFFER) return false;

		glBindBuffer(_baseBuffer.bufferType, _baseBuffer.handle);

		glBindTexture(native->glTexInfo.target, native->handle);
		glGetTexImage(native->glTexInfo.target, mipSlice, native->glTexInfo.format, native->glTexInfo.type, nullptr);

		glBindBuffer(_baseBuffer.bufferType, 0);
		_baseBuffer.doSync<true>();

		return true;
	}

	bool PixelBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void PixelBuffer::destroy() {
		_baseBuffer.releaseBuffer();
	}
}