#pragma once

#include "aurora/ByteArray.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include <unordered_map>

namespace aurora {
	class AE_DLL VertexResource : public Ref {
	public:
		modules::graphics::VertexFormat format;
		ByteArray data;
	};


	class AE_DLL IndexResource : public Ref {
	public:
		modules::graphics::IndexType type;
		ByteArray data;
	};


	class AE_DLL MeshResource : public Ref {
	public:
		inline const std::unordered_map<std::string, RefPtr<VertexResource>>& AE_CALL getVertexResources() const {
			return _vertices;
		}

		inline void AE_CALL setVertexSource(const std::string& name, VertexResource* res) {
			if (res) {
				_vertices.insert_or_assign(name, res);
			} else {
				_vertices.erase(name);
			}
		}

		std::string name;
		RefPtr<IndexResource> indexResource;

	private:
		std::unordered_map<std::string, RefPtr<VertexResource>> _vertices;
	};
}