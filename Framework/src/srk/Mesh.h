#pragma once

#include "srk/GraphicsBuffer.h"

namespace srk {
	class SRK_FW_DLL VertexResource : public Ref {
	public:
		uint32_t stride = 0;
		ByteArray data;
	};


	class SRK_FW_DLL IndexResource : public Ref {
	public:
		modules::graphics::IndexType type = modules::graphics::IndexType::UNKNOWN;
		ByteArray data;
	};


	class SRK_FW_DLL MeshResource : public Ref {
	public:
		inline const auto& SRK_CALL getVerteices() const {
			return _vertices;
		}

		inline std::optional<modules::graphics::VertexAttribute<VertexResource>> SRK_CALL getVertex(const std::string_view& name) const {
			auto itr = _vertices.find(name);
			return itr == _vertices.end() ? std::nullopt : std::make_optional(itr->second);
		}

		void SRK_CALL setVertex(const std::string_view& name, const modules::graphics::VertexAttribute<VertexResource>& attrib);

		inline std::optional<modules::graphics::VertexAttribute<VertexResource>> SRK_CALL remove(const std::string_view& name) {
			return _remove(name);
		}

		std::string name;
		IntrusivePtr<IndexResource> index;

	private:
#ifdef __cpp_lib_generic_unordered_lookup
		struct MapHasher {
			using is_transparent = void;
			template<typename K> inline size_t SRK_CALL operator()(K&& key) const { return std::hash<std::string_view>{}(key); }
		};
		std::unordered_map<std::string, modules::graphics::VertexAttribute<VertexResource>, MapHasher, std::equal_to<>>
#else
		std::map<std::string, modules::graphics::VertexAttribute<VertexResource>, std::less<>>
#endif
		_vertices;

		std::optional<modules::graphics::VertexAttribute<VertexResource>> SRK_CALL _remove(const std::string_view& name);
	};


	class SRK_FW_DLL MeshBuffer : public Ref {
	public:
		MeshBuffer();

		inline IntrusivePtr<VertexAttributeCollection> SRK_CALL getVertices() {
			return _vertices;
		}

		inline const IntrusivePtr<VertexAttributeCollection>& SRK_CALL getVertices() const {
			return _vertices;
		}

		inline void SRK_CALL setVertices(VertexAttributeCollection* buffers) {
			_vertices = buffers;
		}

		inline const IntrusivePtr<modules::graphics::IIndexBuffer>& SRK_CALL getIndex() const {
			return _index;
		}

		inline void SRK_CALL setIndex(modules::graphics::IIndexBuffer* buffer) {
			_index = buffer;
		}

	private:
		IntrusivePtr<VertexAttributeCollection> _vertices;
		IntrusivePtr<modules::graphics::IIndexBuffer> _index;
	};


	class SRK_FW_DLL Mesh : public Ref {
	public:
		inline const IntrusivePtr<MeshBuffer>& SRK_CALL getBuffer() const {
			return _buffer;
		}

		inline void SRK_CALL setBuffer(MeshBuffer* buffer) {
			_buffer = buffer;
		}

		inline const IntrusivePtr<MeshResource>& SRK_CALL getResource() const {
			return _resource;
		}

		inline void SRK_CALL setResource(MeshResource* resource) {
			_resource = resource;
		}

	protected:
		IntrusivePtr<MeshBuffer> _buffer;
		IntrusivePtr<MeshResource> _resource;
	};
}