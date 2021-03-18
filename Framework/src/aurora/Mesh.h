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

		inline IntrusivePtr<VertexResource> AE_CALL getVertex(const QueryString& name) const {
			auto itr = _vertices.find(name);
			return itr == _vertices.end() ? nullptr : itr->second;
		}

		void AE_CALL setVertex(const QueryString& name, VertexResource* res);
		inline IntrusivePtr<VertexResource> AE_CALL remove(const QueryString& name) {
			return _remove(name);
		}

		std::string name;
		IntrusivePtr<IndexResource> index;

	private:
		StringUnorderedMap<IntrusivePtr<VertexResource>> _vertices;

		VertexResource* AE_CALL _remove(const QueryString& name);
	};


	class AE_FW_DLL MeshBuffer : public Ref {
	public:
		MeshBuffer();

		inline IntrusivePtr<VertexBufferCollection> AE_CALL getVertices() {
			return _vertices;
		}

		inline const IntrusivePtr<VertexBufferCollection>& AE_CALL getVertices() const {
			return _vertices;
		}

		inline void AE_CALL setVertices(VertexBufferCollection* buffers) {
			_vertices = buffers;
		}

		inline const IntrusivePtr<modules::graphics::IIndexBuffer>& AE_CALL getIndex() const {
			return _index;
		}

		inline void AE_CALL setIndex(modules::graphics::IIndexBuffer* buffer) {
			_index = buffer;
		}

	private:
		IntrusivePtr<VertexBufferCollection> _vertices;
		IntrusivePtr<modules::graphics::IIndexBuffer> _index;
	};


	class AE_FW_DLL Mesh : public Ref {
	public:
		inline const IntrusivePtr<MeshBuffer>& AE_CALL getBuffer() const {
			return _buffer;
		}

		inline void AE_CALL setBuffer(MeshBuffer* buffer) {
			_buffer = buffer;
		}

		inline const IntrusivePtr<MeshResource>& AE_CALL getResource() const {
			return _resource;
		}

		inline void AE_CALL setResource(MeshResource* resource) {
			_resource = resource;
		}

	protected:
		IntrusivePtr<MeshBuffer> _buffer;
		IntrusivePtr<MeshResource> _resource;
	};
}