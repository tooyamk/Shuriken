#include "VertexBufferFactory.h"
#include "modules/graphics/IGraphicsModule.h"

namespace aurora::modules::graphics {
	VertexBufferFactory::~VertexBufferFactory() {
		clear();
	}

	IVertexBuffer* VertexBufferFactory::get(const std::string& name) const {
		auto itr = _buffers.find(name);
		return itr == _buffers.end() ? nullptr : itr->second;
	}

	void VertexBufferFactory::add(const std::string& name, IVertexBuffer* buffer) {
		auto itr = _buffers.find(name);
		if (buffer) {
			if (itr == _buffers.end()) {
				buffer->ref();
				_buffers.emplace(name, buffer);
			} else if (itr->second != buffer) {
				buffer->ref();
				itr->second->unref();
				itr->second = buffer;
			}
		} else {
			if (itr != _buffers.end()) {
				itr->second->unref();
				_buffers.erase(itr);
			}
		}
	}

	void VertexBufferFactory::remove(const std::string& name) {
		auto itr = _buffers.find(name);
		if (itr != _buffers.end()) {
			itr->second->unref();
			_buffers.erase(itr);
		}
	}

	void VertexBufferFactory::clear() {
		for (auto& itr : _buffers) itr.second->unref();
		_buffers.clear();
	}
}