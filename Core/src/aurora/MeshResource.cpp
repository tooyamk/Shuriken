#include "MeshResource.h"

namespace aurora {
	void MeshResource::setVertexSource(const std::string& name, VertexResource* res) {
		auto itr = _vertices.find(name);
		if (itr == _vertices.end()) {
			if (res) _vertices.emplace(name, res);
		} else if (itr->second != res) {
			if (res) {
				itr->second = res;
			} else {
				_vertices.erase(itr);
			}
		}
	}
}