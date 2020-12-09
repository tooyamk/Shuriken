#pragma once

#include "aurora/GraphicsBuffer.h"

namespace aurora {
	class AE_FW_DLL VertexResource : public Ref {
	public:
		modules::graphics::VertexFormat format;
		ByteArray data;
	};


	class AE_FW_DLL IndexResource : public Ref {
	public:
		modules::graphics::IndexType type = modules::graphics::IndexType::UNKNOWN;
		ByteArray data;
	};


	class AE_FW_DLL MeshResource : public Ref {
	public:
		inline const std::unordered_map<std::string, RefPtr<VertexResource>>& AE_CALL getVerteices() const {
			return _vertices;
		}

		inline VertexResource* AE_CALL getVertex(const std::string& name) const {
			auto itr = _vertices.find(name);
			return itr == _vertices.end() ? nullptr : itr->second;
		}

		inline void AE_CALL setVertex(const std::string& name, VertexResource* res) {
			if (res) {
				_vertices.insert_or_assign(name, res);
			} else {
				_vertices.erase(name);
			}
		}

		std::string name;
		RefPtr<IndexResource> index;

	private:
		std::unordered_map<std::string, RefPtr<VertexResource>> _vertices;
	};


	class AE_FW_DLL MeshBuffer : public Ref {
	public:
		MeshBuffer();

		inline RefPtr<VertexBufferCollection> AE_CALL getVertices() {
			return _vertices;
		}

		inline const RefPtr<VertexBufferCollection>& AE_CALL getVertices() const {
			return _vertices;
		}

		inline void AE_CALL setVertices(VertexBufferCollection* buffers) {
			_vertices = buffers;
		}

		inline const RefPtr<modules::graphics::IIndexBuffer>& AE_CALL getIndex() const {
			return _index;
		}

		inline void AE_CALL setIndex(modules::graphics::IIndexBuffer* buffer) {
			_index = buffer;
		}

	private:
		RefPtr<VertexBufferCollection> _vertices;
		RefPtr<modules::graphics::IIndexBuffer> _index;
	};


	class AE_FW_DLL Mesh : public Ref {
	public:
		inline const RefPtr<MeshBuffer>& AE_CALL getBuffer() const {
			return _buffer;
		}

		inline void AE_CALL setBuffer(MeshBuffer* buffer) {
			_buffer = buffer;
		}

		inline const RefPtr<MeshResource>& AE_CALL getResource() const {
			return _resource;
		}

		inline void AE_CALL setResource(MeshResource* resource) {
			_resource = resource;
		}

	protected:
		RefPtr<MeshBuffer> _buffer;
		RefPtr<MeshResource> _resource;
	};
}