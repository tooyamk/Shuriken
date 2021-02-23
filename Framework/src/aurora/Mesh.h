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
		inline const auto& AE_CALL getVerteices() const {
			return _vertices;
		}

		inline RefPtr<VertexResource> AE_CALL getVertex(const query_string& name) const {
			auto itr = _vertices.find(name);
			return itr == _vertices.end() ? nullptr : itr->second;
		}

		void AE_CALL setVertex(const query_string& name, VertexResource* res);
		inline RefPtr<VertexResource> AE_CALL remove(const query_string& name) {
			return _remove(name);
		}

		std::string name;
		RefPtr<IndexResource> index;

	private:
		string_unordered_map<RefPtr<VertexResource>> _vertices;

		VertexResource* AE_CALL _remove(const query_string& name);
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