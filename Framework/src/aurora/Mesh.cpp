#include "Mesh.h"

namespace aurora {
	MeshBuffer::MeshBuffer() :
		_vertices(new VertexBufferCollection()) {
	}


	void AE_CALL MeshResource::setVertex(const query_string& name, VertexResource* res) {
		if (res) {
			if (auto itr = _vertices.find(name); itr == _vertices.end()) {
				_vertices.emplace(name, res);
			} else {
				itr->second = res;
			}
		} else {
			remove(name);
		}
	}

	VertexResource* MeshResource::_remove(const query_string& name) {
		if (auto itr = _vertices.find(name); itr == _vertices.end()) {
			auto val = itr->second;
			_vertices.erase(itr);
			return val;
		}

		return nullptr;
	}
}