#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_glew {
	BaseBuffer::BaseBuffer(GLenum target) :
		_dirty(false),
		_target(target),
		_size(0),
		_handle(0),
		_mapData(nullptr),
		_sync(nullptr) {
	}

	BaseBuffer::~BaseBuffer() {
		_delBuffer();
	}

	bool BaseBuffer::_stroage(ui32 size, const void* data) {
		_delBuffer();

		glGenBuffers(1, &_handle);

		if (_handle) {
			_size = size;

			GLbitfield flags = GL_MAP_WRITE_BIT
				| GL_MAP_PERSISTENT_BIT //在被映射状态下不同步
				| GL_MAP_COHERENT_BIT;  //数据对GPU立即可见

			glBindBuffer(_target, _handle);
			glBufferStorage(_target, size, data, flags);
			_mapData = glMapBufferRange(_target, 0, size, flags);

			return true;
		}

		return false;
	}

	void BaseBuffer::_write(ui32 offset, const void* data, ui32 length) {
		if (_handle && _mapData && data && length && offset < _size) {
			_dirty = true;
			memcpy((i8*)_mapData + offset, data, std::min<ui32>(length, _size - offset));
		}
	}

	void BaseBuffer::_flush() {
		if (_dirty) {
			_waitServerSync();
			_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			_dirty = false;
		}
	}

	void BaseBuffer::_delBuffer() {
		if (_handle) {
			_delSync();

			if (_mapData) {
				glBindBuffer(_target, _handle);
				glUnmapBuffer(_target);
				_mapData = nullptr;
			}

			glDeleteBuffers(1, &_handle);
			_handle = 0;

			_dirty = false;
		}
		_size = 0;
	}

	void BaseBuffer::_waitServerSync() {
		if (_sync) {
			do {
				auto rst = glClientWaitSync(_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (rst == GL_ALREADY_SIGNALED || rst == GL_CONDITION_SATISFIED) {
					_delSync();
					return;
				}

			} while (true);
		}
	}

	void BaseBuffer::_delSync() {
		if (_sync) {
			glDeleteSync(_sync);
			_sync = nullptr;
		}
	}
}