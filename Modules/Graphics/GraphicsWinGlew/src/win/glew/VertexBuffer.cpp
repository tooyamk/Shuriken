#include "VertexBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win::glew {
	VertexBuffer::VertexBuffer(Graphics& graphics) : aurora::modules::graphics::VertexBuffer(graphics),
		_dirty(false),
		_size(0),
		_handle(0),
		_mapData(nullptr),
		_sync(nullptr) {
	}

	VertexBuffer::~VertexBuffer() {
		_delBuffer();
	}

	bool VertexBuffer::stroage(ui32 size, const void* data) {
		_delBuffer();

		glGenBuffers(1, &_handle);

		if (_handle) {
			_size = size;

			GLbitfield flags = GL_MAP_WRITE_BIT
				| GL_MAP_PERSISTENT_BIT //在被映射状态下不同步
				| GL_MAP_COHERENT_BIT;  //数据对GPU立即可见

			glBindBuffer(GL_ARRAY_BUFFER, _handle);
			glBufferStorage(GL_ARRAY_BUFFER, size, data, flags);
			_mapData = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, flags);

			return true;
		}

		return false;
	}

	void VertexBuffer::write(ui32 offset, const void* data, ui32 length) {
		if (_handle && _mapData && data && length && offset < _size) {
			_dirty = true;
			memcpy((i8*)_mapData + offset, data, std::min<ui32>(length, _size - offset));
		}
	}

	void VertexBuffer::flush() {
		if (_dirty) {
			_waitServerSync();
			_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			_dirty = false;
		}
	}

	void VertexBuffer::use() {
		if (_handle) {
			_waitServerSync();

			glBindBuffer(GL_ARRAY_BUFFER, _handle);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
	}

	void VertexBuffer::_delBuffer() {
		if (_handle) {
			_delSync();

			if (_mapData) {
				glBindBuffer(GL_ARRAY_BUFFER, _handle);
				glUnmapBuffer(GL_ARRAY_BUFFER);
				_mapData = nullptr;
			}

			glDeleteBuffers(1, &_handle);
			_handle = 0;

			_dirty = false;
		}
	}

	void VertexBuffer::_waitServerSync() {
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

	void VertexBuffer::_delSync() {
		if (_sync) {
			glDeleteSync(_sync);
			_sync = nullptr;
		}
	}
}