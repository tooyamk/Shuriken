#include "Mesh.h"

namespace srk {
	MeshBuffer::MeshBuffer() :
		_vertices(new VertexAttributeCollection()) {
	}


	void SRK_CALL MeshResource::setVertex(const std::string_view& name, const modules::graphics::VertexAttribute<VertexResource>& attrib) {
		if (auto itr = _vertices.find(name); itr == _vertices.end()) {
			_vertices.emplace(name, attrib);
		} else {
			itr->second = attrib;
		}
	}

	std::optional<modules::graphics::VertexAttribute<VertexResource>> MeshResource::_remove(const std::string_view& name) {
		if (auto itr = _vertices.find(name); itr != _vertices.end()) {
			auto opt = std::make_optional(std::move(itr->second));
			_vertices.erase(itr);
			return opt;
		}

		return std::nullopt;
	}
}