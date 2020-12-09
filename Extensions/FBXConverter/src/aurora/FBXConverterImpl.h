#pragma once

#include "FBXConverter.h"
#include "aurora/ByteArray.h"
#include "aurora/Debug.h"
#include "aurora/Mesh.h"
#include "aurora/ShaderPredefine.h"
#include "boost/preprocessor.hpp"
#include "zlib.h"
#include <vector>
#include <unordered_map>

namespace aurora::extensions::fbx_converter {
#define AE_FBX_CONVERTER_TO_STR_ARRAY_ELEM(r, data, i, elem) \
    BOOST_PP_COMMA_IF(i) BOOST_PP_STRINGIZE(elem)

#define AE_FBX_CONVERTER_TO_STR_ARRAY(...) \
    BOOST_PP_SEQ_FOR_EACH_I(AE_FBX_CONVERTER_TO_STR_ARRAY_ELEM, ~, BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__)))


#define AE_FBX_CONVERTER_DECLARE_ENUM(__NAME__, __BASE__, ...) \
enum class __NAME__ : __BASE__  {__VA_ARGS__}; \
inline static const std::vector<std::string> __NAME__##_Members = { AE_FBX_CONVERTER_TO_STR_ARRAY(__VA_ARGS__) }; \
inline static std::unordered_map<std::string_view, __NAME__> __NAME__##_RefDataGenerator() { \
	std::unordered_map<std::string_view, __NAME__> map; \
	for (__BASE__ i = 1, n = __NAME__##_Members.size(); i < n; ++i) map.emplace(__NAME__##_Members[i], (__NAME__)i); \
	return map; \
} \
inline static const std::unordered_map<std::string_view, __NAME__> __NAME__##_ref = __NAME__##_RefDataGenerator(); \
inline static __NAME__ get##__NAME__(const std::string_view& name) { \
	auto itr = __NAME__##_ref.find(name); \
	return itr == __NAME__##_ref.end() ? (__NAME__)0 : itr->second; \
}

	class Node {
	public:
		AE_FBX_CONVERTER_DECLARE_ENUM(Type, uint8_t,
			Unknown,

			AnimationCurve,
			AnimationCurveNode,
			AnimationLayer,
			AnimationStack,
			BlendWeights,
			C,
			Connections,
			Deformer,
			Geometry,
			GlobalSettings,
			Indexes,
			KeyTime,
			KeyValueFloat,
			LayerElementBinormal,
			LayerElementNormal,
			LayerElementTangent,
			LayerElementUV,
			MappingInformationType,
			Matrix,
			Model,
			Node,
			Normals,
			Tangents,
			Binormals,
			Objects,
			P,
			PolygonVertexIndex,
			Pose,
			PoseNode,
			Properties70,
			ReferenceInformationType,
			Transform,
			TransformLink,
			UV,
			UVIndex,
			Vertices,
			Weights
		);

		AE_FBX_CONVERTER_DECLARE_ENUM(AttribType, uint8_t,
			Unknown,

			Cluster,
			LimbNode,
			Root,
			Null,
			Mesh,
			Skin,
			T,
			R,
			S
		);

		class iterator {
		public:
			iterator(Node* node = nullptr) :
				_node(node) {
			}

			inline iterator& operator=(const iterator& itr) {
				_node = itr._node;
				return *this;
			}

			inline bool operator==(const iterator& itr) const {
				return _node == itr._node;
			}

			inline bool AE_CALL operator!=(const iterator& itr) const {
				return _node != itr._node;
			}

			inline iterator& AE_CALL operator++() {
				if (_node) _node = _node->_next;
				return *this;
			}

			inline iterator AE_CALL operator++(int32_t) const {
				return iterator(_node ? _node->_next : nullptr);
			}

			inline Node*& AE_CALL operator*() {
				return _node;
			}

		private:
			Node* _node;

			friend Node;
		};

		class Property {
		public:
			enum class Type {
				UNKNOWN,
				BOOL = 'C',
				I16 = 'Y',
				I32 = 'I',
				I64 = 'L',
				F32 = 'F',
				F64 = 'D',
				STR = 'S',
				BOOL_ARR = 'c',
				BOOL_ARR_v2 = 'b',
				I32_ARR = 'i',
				I64_ARR = 'l',
				F32_ARR = 'f',
				F64_ARR = 'd',
				BYTES = 'R'
			};

			struct Value {
				inline static const std::string UpAxis = "UpAxis";
				inline static const std::string UpAxisSign = "UpAxisSign";
				inline static const std::string FrontAxis = "FrontAxis";
				inline static const std::string FrontAxisSign = "FrontAxisSign";
				inline static const std::string CoordAxis = "CoordAxis";
				inline static const std::string CoordAxisSign = "CoordAxisSign";
				inline static const std::string OriginUpAxis = "OriginUpAxis";
				inline static const std::string OriginUpAxisSign = "OriginUpAxisSign";
				inline static const std::string Direct = "Direct";
				inline static const std::string IndexToDirect = "IndexToDirect";
				inline static const std::string ByControlVertex = "ByControlVertex";
				inline static const std::string ByPolygonVertex = "ByPolygonVertex";
				inline static const std::string d_X = "d|X";
				inline static const std::string d_Y = "d|Y";
				inline static const std::string d_Z = "d|Z";
				inline static const std::string GeometricTranslation = "GeometricTranslation";
				inline static const std::string GeometricRotation = "GeometricRotation";
				inline static const std::string GeometricScaling = "GeometricScaling";
				inline static const std::string PreRotation = "PreRotation";
				inline static const std::string RotationActive = "RotationActive";
				inline static const std::string InheritType = "InheritType";
				inline static const std::string ScalingMax = "ScalingMax";
				inline static const std::string DefaultAttributeIndex = "DefaultAttributeIndex";
				inline static const std::string Lcl_Translation = "Lcl Translation";
				inline static const std::string Lcl_Rotation = "Lcl Rotation";
				inline static const std::string Lcl_Scaling = "Lcl Scaling";
				inline static const std::string MaxHandle = "MaxHandle";
				inline static const std::string LocalStart = "LocalStart";
				inline static const std::string LocalStop = "LocalStop";
				inline static const std::string ReferenceStart = "ReferenceStart";
				inline static const std::string ReferenceStop = "ReferenceStop";
			};

			Property() :
				rawVal{ nullptr, 0 } {
			}

			Type type;
			union {
				bool boolVal;
				int16_t i16Val;
				int32_t i32Val;
				int64_t i64Val;
				float32_t f32Val;
				float64_t f64Val;
				struct {
					uint8_t* data;
					size_t size;
				} rawVal;
			};
		};

		Node(Type type, Property* properties, uint16_t numProperties) :
			_type(type),
			_attribType(AttribType::Unknown),
			_numProperties(numProperties),
			_next(nullptr),
			_childHead(nullptr),
			_childTail(nullptr),
			_properties(properties),
			_id(0) {
			if (numProperties) {
				if (numProperties > 0) {
					if (auto& p = _properties[0]; p.type == Property::Type::I64) _id = p.i64Val;
				}
				if (numProperties > 1) {
					if (auto& p = _properties[1]; p.type == Property::Type::STR) _attribName = std::string_view((char*)p.rawVal.data, p.rawVal.size);
				}
				if (numProperties > 2) {
					if (auto& p = _properties[2]; p.type == Property::Type::STR) _attribType = getAttribType(std::string_view((char*)p.rawVal.data, p.rawVal.size));
				}
			}
		}

		~Node() {
			auto c = _childHead;
			while (c) {
				auto next = c->_next;
				delete c;
				c = next;
			}

			if (_properties) delete[] _properties;
		}

		inline iterator AE_CALL begin() const {
			return iterator(_childHead);
		}
		inline iterator AE_CALL end() const {
			return iterator();
		}

		inline Type AE_CALL getType() const {
			return _type;
		}

		inline int64_t AE_CALL getID() const {
			return _id;
		}

		inline const std::string_view& AE_CALL getAttribName() const {
			return _attribName;
		}

		inline AttribType AE_CALL getAttribType() const {
			return _attribType;
		}

		inline uint16_t AE_CALL numProperties() const {
			return _numProperties;
		}

		inline Property* AE_CALL getProperties() {
			return _properties;
		}

		inline const Property* AE_CALL getProperty(Property::Type pt) const {
			for (size_t i = 0; i < _numProperties; ++i) {
				if (_properties[i].type == pt) return &_properties[i];
			}
			return nullptr;
		}

		void AE_CALL addChild(Node* child) {
			if (_childHead) {
				_childTail->_next = child;
				_childTail = child;
			} else {
				_childHead = child;
				_childTail = child;
			}
		}

		Node* AE_CALL getChild(Type type) {
			auto n = _childHead;
			while (n) {
				if (n->_type == type) return n;
				n = n->_next;
			}

			return nullptr;
		}

	private:
		Type _type;
		AttribType _attribType;
		uint16_t _numProperties;

		std::string_view _attribName;

		Node* _next;

		Node* _childHead;
		Node* _childTail;

		Property* _properties;

		int64_t _id;
	};

	class Connection {
	public:
		Connection(int64_t id, const std::string_view& relationship) :
			id(id),
			relationship(relationship) {
		}

		const int64_t id;
		const std::string_view relationship;
	};

	class SkeletonData {
	public:

	};

	class FBX {
	public:
		using ConnectionType = std::unordered_map<int64_t, std::vector<Connection>>;

		FBX(uint32_t ver) :
			ver(ver),
			root(Node::Type::Unknown, nullptr, 0),
			globalSettings(nullptr),
			_rightHanded(false) {
		}

		~FBX() {
			for (auto ba : allocatedData) delete ba;
		}

		uint32_t ver;
		Node root;
		std::vector<ByteArray*> allocatedData;

		std::unordered_map<int64_t, Node*> objects;
		std::vector<Node*> animationStacks;
		std::vector<Node*> models;
		std::vector<Node*> poses;
		Node* globalSettings;

		ConnectionType connectionChildren;
		ConnectionType connectionParents;

		void AE_CALL addNode(Node* node) {
			switch (node->getType()) {
			case Node::Type::AnimationStack:
			{
				objects.try_emplace(node->getID(), node);
				animationStacks.emplace_back(node);

				break;
			}
			case Node::Type::AnimationLayer:
			case Node::Type::AnimationCurveNode:
			case Node::Type::AnimationCurve:
			case Node::Type::Geometry:
			case Node::Type::Deformer:
				objects.try_emplace(node->getID(), node);
				break;
			case Node::Type::Model:
			{
				objects.try_emplace(node->getID(), node);
				models.emplace_back(node);

				break;
			}
			case Node::Type::Pose:
			{
				objects.try_emplace(node->getID(), node);
				poses.emplace_back(node);

				break;
			}
			case Node::Type::C:
			case Node::Type::Connections:
			{
				if (node->numProperties() > 2) {
					auto properties = node->getProperties();
					if (auto& p1 = properties[1]; p1.type == Node::Property::Type::I64 && p1.i64Val) {
						if (auto& p2 = properties[2]; p2.type == Node::Property::Type::I64) {
							std::string_view relationship;
							if (node->numProperties() > 3) {
								if (auto& p3 = properties[3]; p3.type == Node::Property::Type::STR) relationship = std::string_view((char*)p3.rawVal.data, p3.rawVal.size);
							}

							_addConnection(connectionChildren, p2.i64Val, p1.i64Val, relationship);
							_addConnection(connectionParents, p1.i64Val, p2.i64Val, relationship);
						}
					}
				}

				break;
			}
			case Node::Type::GlobalSettings:
				globalSettings = node;
				break;
			default:
				break;
			}
		}

		inline Node* AE_CALL getNode(int64_t id) const {
			auto itr = objects.find(id);
			return itr == objects.end() ? nullptr : itr->second;
		}

		inline std::vector<Connection>* AE_CALL getConnectionChildren(int64_t id) {
			auto itr = connectionChildren.find(id);
			return itr == connectionChildren.end() ? nullptr : &itr->second;
		}

		inline std::vector<Connection>* AE_CALL getConnectionParents(int64_t id) {
			auto itr = connectionParents.find(id);
			return itr == connectionParents.end() ? nullptr : &itr->second;
		}

		Node* AE_CALL findConnectionChildNode(int64_t id, Node::Type type) {
			if (auto itr = connectionChildren.find(id); itr != connectionChildren.end()) {
				for (auto& c : itr->second) {
					if (auto itr2 = objects.find(c.id); itr2 != objects.end()) {
						auto o = itr2->second;
						if (o->getType() == type) return o;
					}
				}
			}
			return nullptr;
		}

		void AE_CALL build(FBXConverter::Result& rst) {
			_buildGlobalSettings();

			std::vector<Node*> meshes;
			SkeletonData skeleton;

			if (!poses.empty()) {
				//todo
			}

			if (!models.empty()) {
				for (auto m : models) {
					switch (m->getAttribType()) {
					case Node::AttribType::Mesh:
						meshes.emplace_back(m);
						break;
					case Node::AttribType::LimbNode:
					case Node::AttribType::Root:
					{
						//todo

						break;
					}
					default:
						break;
					}
				}
			}

			if (!meshes.empty()) {
				for (auto m : meshes) {
					if (auto mesh = _buildGeometry(*m, skeleton); mesh) {
						rst.meshes.emplace_back(mesh);

						if (_rightHanded) {
							for (auto& itr : mesh->getVerteices()) {
								auto& vr = itr.second;
								if (vr->format.size == modules::graphics::VertexSize::THREE) {
									switch (vr->format.type) {
									case modules::graphics::VertexType::I16:
									case modules::graphics::VertexType::UI16:
										_transformXZY<2>(vr->data);
										break;
									case modules::graphics::VertexType::I32:
									case modules::graphics::VertexType::UI32:
									case modules::graphics::VertexType::F32:
										_transformXZY<4>(vr->data);
										break;
									default:
										break;
									}
								}
							}

							if (mesh->index) {
								switch (mesh->index->type) {
								case modules::graphics::IndexType::UI16:
									_transformXZY<2>(mesh->index->data);
									break;
								case modules::graphics::IndexType::UI32:
									_transformXZY<4>(mesh->index->data);
									break;
								default:
									break;
								}
							}
						}
					}
				}
			}
		}

	private:
		bool _rightHanded;

		void AE_CALL _addConnection(ConnectionType& connections, int64_t key, int64_t id, const std::string_view& relationship) {
			connections.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple()).first->second.emplace_back(id, relationship);
		}

		template<size_t Size>
		void AE_CALL _transformXZY(ByteArray& data) {
			if (auto src = data.getSource(); src) {
				constexpr size_t STEP = Size * 3;
				uint8_t tmp[Size];
				for (size_t i = 0, n = data.getLength() / STEP; i < n; ++i) {
					auto y = src + Size;
					auto z = y + Size;
					memcpy(tmp, z, Size);
					memcpy(z, y, Size);
					memcpy(y, tmp, Size);

					src += STEP;
				}
			}
		}

		void AE_CALL _buildGlobalSettings() {
			if (globalSettings) {
				if (auto n = globalSettings->getChild(Node::Type::Properties70); n) {
					for (auto c : *n) {
						if (c->getType() != Node::Type::P || !c->numProperties()) continue;

						if (auto& p = c->getProperties()[0]; p.type == Node::Property::Type::STR) {
							auto& pp = c->getProperties()[c->numProperties() - 1];

							std::string_view val((char*)p.rawVal.data, p.rawVal.size);
							if (val == Node::Property::Value::UpAxis) {
								int a = 1;
							} else if (val == Node::Property::Value::UpAxisSign) {
								int a = 1;
							} else if (val == Node::Property::Value::FrontAxis) {
								int a = 1;
							} else if (val == Node::Property::Value::FrontAxisSign) {
								int a = 1;
							} else if (val == Node::Property::Value::CoordAxis) {
								if (pp.type == Node::Property::Type::I16 || pp.type == Node::Property::Type::I32 || pp.type == Node::Property::Type::I64) {
									_rightHanded = pp.i64Val == 0;
								}
							} else if (val == Node::Property::Value::CoordAxisSign) {
								int a = 1;
							}
						}
					}
				}
			}
		}

		MeshResource* AE_CALL _buildGeometry(Node& node, SkeletonData& skeleton) {
			if (auto geometry = findConnectionChildNode(node.getID(), Node::Type::Geometry); geometry) {
				if (auto child = geometry->getChild(Node::Type::PolygonVertexIndex); child) {
					std::vector<uint32_t> sourceIndices;
					std::vector<uint8_t> faces;
					if (auto [triangulate, numTriangulateIndices] = _buildPolygonVertexIndex(*child, sourceIndices, faces); !sourceIndices.empty()) {
						auto mesh = new MeshResource();
						mesh->name = node.getAttribName();

						for (auto c : *geometry) {
							switch (c->getType()) {
							case Node::Type::Vertices:
							{
								_buildPosition(*c, *mesh, sourceIndices, faces, triangulate, numTriangulateIndices);

								break;
							}
							case Node::Type::LayerElementNormal:
								_buildOtherVertices<Node::Type::Normals>(*c, *mesh, ShaderPredefine::NORMAL0, sourceIndices);
								break;
							case Node::Type::LayerElementTangent:
								_buildOtherVertices<Node::Type::Tangents>(*c, *mesh, ShaderPredefine::TANGENT0, sourceIndices);
								break;
							case Node::Type::LayerElementBinormal:
								_buildOtherVertices<Node::Type::Binormals>(*c, *mesh, ShaderPredefine::BINORMAL0, sourceIndices);
								break;
							case Node::Type::LayerElementUV:
								_buildUVs(*c, *mesh, sourceIndices);
								break;
							default:
								break;
							}
						}

						return mesh;
					}
				}
			}

			return nullptr;
		}

		std::tuple<bool, size_t> AE_CALL _buildPolygonVertexIndex(Node& node, std::vector<uint32_t>& sourceIndices, std::vector<uint8_t>& faces) {
			auto triangulate = false;
			size_t numTriangulateIndices = 0;

			if (node.numProperties() > 0) {
				if (auto& p = node.getProperties()[0]; p.type == Node::Property::Type::I32_ARR) {
					auto count = p.rawVal.size;
					ByteArray src(p.rawVal.data, count << 2, ByteArray::Usage::SHARED);

					size_t start = 0;
					sourceIndices.resize(count);
					for (size_t i = 0; i < count; ++i) {
						auto idx = src.read<ba_vt::I32>();
						if (idx < 0) {
							idx = ~idx;
							auto n = i - start + 1;
							if (n == 3) {
								numTriangulateIndices += 3;
							} else {
								triangulate = true;
								if (n > 3) {
									numTriangulateIndices += (n - 2) * 3;
								} else {
									numTriangulateIndices += 3;
								}
							}
							faces.emplace_back(n);
							start = i + 1;
						}

						sourceIndices[i] = idx;
					}
				}
			}

			return std::make_tuple(triangulate, numTriangulateIndices);
		}

		void AE_CALL _buildPosition(Node& node, MeshResource& mesh, const std::vector<uint32_t>& sourceIndices, const std::vector<uint8_t>& faces, bool triangulate, size_t numTriangulateIndices) {
			if (node.numProperties() > 0) {
				if (auto& p = node.getProperties()[0]; p.type == Node::Property::Type::F32_ARR || p.type == Node::Property::Type::F64_ARR) {
					auto vs = new VertexResource();
					vs->format.size = modules::graphics::VertexSize::THREE;
					vs->format.type = modules::graphics::VertexType::F32;
					if (p.type == Node::Property::Type::F32_ARR) {
						vs->data = _buildPosition<float32_t, float32_t>(p, sourceIndices);
					} else {
						vs->data = _buildPosition<float64_t, float32_t>(p, sourceIndices);
					}

					mesh.setVertex(ShaderPredefine::POSITION0, vs);

					if (auto count = sourceIndices.size(); p.rawVal.size <= BitUInt<16>::MAX) {
						_buildIndices<uint16_t>(mesh, faces, count, triangulate, numTriangulateIndices);
					} else {
						_buildIndices<uint32_t>(mesh, faces, count, triangulate, numTriangulateIndices);
					}
				}
			}
		}

		template<typename SrcType, typename DstType>
		ByteArray AE_CALL _buildPosition(const Node::Property& p, const std::vector<uint32_t>& sourceIndices) {
			auto count = sourceIndices.size();

			if constexpr (std::is_same_v<SrcType, DstType>) {
				auto src = p.rawVal.data;

				constexpr size_t VERTEX_SIZE = sizeof(DstType) * 3;
				ByteArray dst(VERTEX_SIZE * count);
				dst.setLength(dst.getCapacity());

				size_t dstOffset = 0;
				for (size_t i = 0; i < count; ++i) {
					memcpy(dst.getSource() + dstOffset, src + sourceIndices[i] * VERTEX_SIZE, VERTEX_SIZE);
					dstOffset += VERTEX_SIZE;
				}

				return std::move(dst);
			} else {
				ByteArray src(p.rawVal.data, p.rawVal.size * sizeof(SrcType), ByteArray::Usage::SHARED);
				ByteArray dst(count * sizeof(DstType) * 3);

				constexpr size_t VERTEX_SIZE = sizeof(SrcType) * 3;

				size_t dstOffset = 0;
				for (size_t i = 0; i < count; ++i) {
					src.setPosition(sourceIndices[i] * VERTEX_SIZE);

					dst.write<DstType>(src.read<SrcType>());
					dst.write<DstType>(src.read<SrcType>());
					dst.write<DstType>(src.read<SrcType>());
				}
				dst.seekBegin();

				return std::move(dst);
			}
		}

		template<typename T>
		void AE_CALL _buildIndices(MeshResource& mesh, const std::vector<uint8_t>& faces, size_t count, bool triangulate, size_t numTriangulateIndices) {
			size_t allocSize = numTriangulateIndices * sizeof(T);

			ByteArray dst(allocSize);

			if (triangulate) {
				size_t srcIdx = 0;
				for (auto i : faces) {
					switch (i) {
					case 0:
						break;
					case 1:
					{
						dst.write<T>(srcIdx);
						dst.write<T>(srcIdx);
						dst.write<T>(srcIdx);
						++srcIdx;

						break;
					}
					case 2:
					{
						dst.write<T>(srcIdx);
						dst.write<T>(++srcIdx);
						dst.write<T>(srcIdx);
						++srcIdx;

						break;
					}
					case 3:
					{
						dst.write<T>(srcIdx);
						dst.write<T>(++srcIdx);
						dst.write<T>(++srcIdx);
						++srcIdx;

						break;
					}
					default:
					{
						auto idx = srcIdx;

						dst.write<T>(idx);
						dst.write<T>(idx + 1);
						dst.write<T>(idx + 2);

						for (size_t j = 1, n = i - 2; j < n; ++j) {
							dst.write<T>(idx);
							dst.write<T>(idx + j + 1);
							dst.write<T>(idx + j + 2);
						}

						srcIdx += i;

						break;
					}
					}
				}
			} else {
				for (size_t i = 0; i < count; ++i) dst.write<T>(i);
			}

			auto is = new IndexResource();
			if constexpr (sizeof(T) == 2) {
				is->type = modules::graphics::IndexType::UI16;
			} else {
				is->type = modules::graphics::IndexType::UI32;
			}
			is->data = std::move(dst);
			mesh.index = is;
		}

		template<Node::Type Type>
		void AE_CALL _buildOtherVertices(Node& node, MeshResource& mesh, const std::string& key, const std::vector<uint32_t>& sourceIndices) {
			const Node::Property* values = nullptr, *refType = nullptr, *mappingType = nullptr;

			for (auto c : node) {
				switch (c->getType()) {
				case Type:
				{
					values = c->getProperty(Node::Property::Type::F32_ARR);
					if (!values) values = c->getProperty(Node::Property::Type::F64_ARR);

					break;
				}
				case Node::Type::ReferenceInformationType:
					refType = c->getProperty(Node::Property::Type::STR);
					break;
				case Node::Type::MappingInformationType:
					mappingType = c->getProperty(Node::Property::Type::STR);
					break;
				default:
					break;
				}
			}

			if (values && refType && mappingType) {
				std::string_view rt((char*)refType->rawVal.data, refType->rawVal.size);
				std::string_view mt((char*)mappingType->rawVal.data, mappingType->rawVal.size);
				VertexResource* vs = nullptr;
				if (values->type == Node::Property::Type::F32_ARR) {
					vs = _buildVertexSource<float32_t, uint32_t, float32_t>(*values, nullptr, rt, mt, sourceIndices, 3);
				} else {
					vs = _buildVertexSource<float64_t, uint32_t, float32_t>(*values, nullptr, rt, mt, sourceIndices, 3);
				}
				if (vs) {
					vs->format.size = modules::graphics::VertexSize::THREE;
					vs->format.type = modules::graphics::VertexType::F32;
					mesh.setVertex(key, vs);
				}
			}
		}

		void AE_CALL _buildUVs(Node& node, MeshResource& mesh, const std::vector<uint32_t>& sourceIndices) {
			const Node::Property* values = nullptr, *indices= nullptr, *refType = nullptr, *mappingType = nullptr;

			for (auto c : node) {
				switch (c->getType()) {
				case Node::Type::UV:
				{
					values = c->getProperty(Node::Property::Type::F32_ARR);
					if (!values) values = c->getProperty(Node::Property::Type::F64_ARR);

					break;
				}
				case Node::Type::UVIndex:
				{
					indices = c->getProperty(Node::Property::Type::I32_ARR);
					if (!indices) indices = c->getProperty(Node::Property::Type::I64_ARR);

					break;
				}
				case Node::Type::ReferenceInformationType:
					refType = c->getProperty(Node::Property::Type::STR);
					break;
				case Node::Type::MappingInformationType:
					mappingType = c->getProperty(Node::Property::Type::STR);
					break;
				default:
					break;
				}
			}

			if (values && indices && refType && mappingType) {
				std::string_view rt((char*)refType->rawVal.data, refType->rawVal.size);
				std::string_view mt((char*)mappingType->rawVal.data, mappingType->rawVal.size);
				VertexResource* vs = nullptr;
				if (values->type == Node::Property::Type::F32_ARR) {
					if (indices->type == Node::Property::Type::I32_ARR) {
						vs = _buildVertexSource<float32_t, uint32_t, float32_t>(*values, indices, rt, mt, sourceIndices, 2);
					} else {
						vs = _buildVertexSource<float32_t, uint64_t, float32_t>(*values, indices, rt, mt, sourceIndices, 2);
					}
				} else {
					if (indices->type == Node::Property::Type::I32_ARR) {
						vs = _buildVertexSource<float64_t, uint32_t, float32_t>(*values, indices, rt, mt, sourceIndices, 2);
					} else {
						vs = _buildVertexSource<float64_t, uint64_t, float32_t>(*values, indices, rt, mt, sourceIndices, 2);
					}
				}
				if (vs) {
					vs->format.size = modules::graphics::VertexSize::TWO;
					vs->format.type = modules::graphics::VertexType::F32;
					mesh.setVertex(ShaderPredefine::UV0, vs);
				}
			}
		}

		template<typename ValueType, typename IndexType, typename DstType>
		VertexResource* AE_CALL _buildVertexSource(const Node::Property& valueProperty, const Node::Property* indexProperty, 
			const std::string_view& refType, const std::string_view& mappingType, const std::vector<uint32_t>& sourceIndices, size_t numDataPerVertex) {
			auto size = sourceIndices.size();
			const auto DST_VERTEX_SIZE = sizeof(DstType) * numDataPerVertex;
			auto values = (ValueType*)valueProperty.rawVal.data;
			ByteArray dst(size * DST_VERTEX_SIZE);

			if (mappingType == Node::Property::Value::ByControlVertex) {
				if (refType == Node::Property::Value::Direct) {
					if constexpr (std::is_same_v<ValueType, DstType>) {
						size_t dstOffset = 0;
						for (size_t i = 0; i < size; ++i) {
							memcpy(dst.getSource() + dstOffset, values + sourceIndices[i] * numDataPerVertex, DST_VERTEX_SIZE);
							dstOffset += DST_VERTEX_SIZE;
						}
						dst.setLength(dstOffset);
					} else {
						for (size_t i = 0; i < size; ++i) {
							auto val = values + sourceIndices[i] * numDataPerVertex;
							for (size_t j = 0; j < numDataPerVertex; ++j) {
								auto v = val[j];
								if constexpr (std::endian::native != std::endian::little) {
									v = byteswap(v);
								}
								dst.write<DstType>(v);
							}
						}
						dst.seekBegin();
					}
				} else if (refType == Node::Property::Value::IndexToDirect && indexProperty) {
					auto indices = (IndexType*)indexProperty->rawVal.data;

					if constexpr (std::is_same_v<ValueType, DstType>) {
						size_t dstOffset = 0;
						for (size_t i = 0; i < size; ++i) {
							auto idx = *(indices + sourceIndices[i]);
							if constexpr (std::endian::native != std::endian::little) {
								idx = byteswap(idx);
							}
							memcpy(dst.getSource() + dstOffset, values + idx * numDataPerVertex, DST_VERTEX_SIZE);
							dstOffset += DST_VERTEX_SIZE;
						}
						dst.setLength(dstOffset);
					} else {
						for (size_t i = 0; i < size; ++i) {
							auto idx = *(indices + sourceIndices[i]);
							if constexpr (std::endian::native != std::endian::little) {
								idx = byteswap(idx);
							}
							auto val = values + idx * numDataPerVertex;
							for (size_t j = 0; j < numDataPerVertex; ++j) {
								auto v = val[j];
								if constexpr (std::endian::native != std::endian::little) {
									v = byteswap(v);
								}
								dst.write<DstType>(v);
							}
						}
						dst.seekBegin();
					}
				} else {
					return nullptr;
				}
			} else if (mappingType == Node::Property::Value::ByPolygonVertex) {
				if (refType == Node::Property::Value::Direct) {
					if constexpr (std::is_same_v<ValueType, DstType>) {
						size_t cpyLen = size * DST_VERTEX_SIZE;
						memcpy(dst.getSource(), values, cpyLen);
						dst.setLength(cpyLen);
					} else {
						for (size_t i = 0, n = size * numDataPerVertex; i < n; ++i) {
							auto v = values[i];
							if constexpr (std::endian::native != std::endian::little) {
								v = byteswap(v);
							}
							dst.write<DstType>(v);
						}
						dst.seekBegin();
					}
				} else if (refType == Node::Property::Value::IndexToDirect && indexProperty) {
					auto indices = (IndexType*)indexProperty->rawVal.data;

					if constexpr (std::is_same_v<ValueType, DstType>) {
						size_t dstOffset = 0;
						for (size_t i = 0; i < size; ++i) {
							auto idx = *(indices + i);
							if constexpr (std::endian::native != std::endian::little) {
								idx = byteswap(idx);
							}
							memcpy(dst.getSource() + dstOffset, values + idx * numDataPerVertex, DST_VERTEX_SIZE);
							dstOffset += DST_VERTEX_SIZE;
						}
						dst.setLength(dstOffset);
					} else {
						for (size_t i = 0; i < size; ++i) {
							auto idx = *(indices + i);
							if constexpr (std::endian::native != std::endian::little) {
								idx = byteswap(idx);
							}
							auto val = values + idx * numDataPerVertex;
							for (size_t j = 0; j < numDataPerVertex; ++j) {
								auto v = val[j];
								if constexpr (std::endian::native != std::endian::little) {
									v = byteswap(v);
								}
								dst.write<DstType>(v);
							}
						}
						dst.seekBegin();
					}
				} else {
					return nullptr;
				}
			} else {
				return nullptr;
			}

			auto vs = new VertexResource();
			vs->data = std::move(dst);

			return vs;
		}

		void AE_CALL _buildSkin() {

		}
	};

	inline bool AE_CALL parseNodeProperty(FBX& fbx, ByteArray& source, Node::Property& p) {
		auto type = (Node::Property::Type)source.read<ba_vt::UI8>();
		p.type = type;
		switch (type) {
		case Node::Property::Type::BOOL:
			p.boolVal = source.read<ba_vt::BOOL>();
			break;
		case Node::Property::Type::I16:
		{
			p.type = Node::Property::Type::I64;
			p.i64Val = source.read<ba_vt::I16>();
			break;
		}
		case Node::Property::Type::I32:
		{
			p.type = Node::Property::Type::I64;
			p.i64Val = source.read<ba_vt::I32>();
			break;
		}
		case Node::Property::Type::I64:
			p.i64Val = source.read<ba_vt::I64>();
			break;
		case Node::Property::Type::F32:
			p.f32Val = source.read<ba_vt::F32>();
			break;
		case Node::Property::Type::F64:
			p.f64Val = source.read<ba_vt::F64>();
			break;
		case Node::Property::Type::BYTES:
		case Node::Property::Type::STR:
		{
			auto size = source.read<ba_vt::UI32>();
			p.rawVal.size = size;
			p.rawVal.data = source.getSource() + source.getPosition();
			source.skip(size);

			break;
		}
		case Node::Property::Type::BOOL_ARR:
		case Node::Property::Type::BOOL_ARR_v2:
		case Node::Property::Type::I32_ARR:
		case Node::Property::Type::I64_ARR:
		case Node::Property::Type::F32_ARR:
		case Node::Property::Type::F64_ARR:
		{
			auto arrSize = source.read<ba_vt::I32>();
			auto encoding = source.read<ba_vt::I32>();
			auto compressedLength = source.read<ba_vt::I32>();

			ByteArray* uncompressData = nullptr;
			if (encoding == 1) {
				size_t inflateVal = 2;
				do {
					size_t capacity = compressedLength << inflateVal;
					uint8_t* data = new uint8_t[capacity];
					uLongf size = capacity;
					if (auto rst = ::uncompress(data, &size, source.getSource() + source.getPosition(), compressedLength); rst == Z_OK) {
						uncompressData = new ByteArray(data, size, capacity, ByteArray::Usage::EXCLUSIVE);
						fbx.allocatedData.emplace_back(uncompressData);

						break;
					} else {
						delete[] data;

						if (rst == Z_BUF_ERROR) {
							++inflateVal;
						} else {
							printdln("FBX parse error : uncompress error ");
							return false;
						}
					}

				} while (true);

				source.skip(compressedLength);
			} else {
				uncompressData = &source;
			}

			if (type == Node::Property::Type::BOOL_ARR_v2) p.type = Node::Property::Type::BOOL_ARR;
			p.rawVal.data = uncompressData->getSource() + uncompressData->getPosition();
			p.rawVal.size = arrSize;

			break;
		}
		default:
			printdln("FBX parse error : Unknown property type ", type);
			return false;
		}

		return true;
	}

	inline bool AE_CALL parseNode(FBX& fbx, ByteArray& source, Node* parent) {
		uint64_t endOffset, numProperties, propertyListLen;
		if (fbx.ver < 7500) {
			endOffset = source.read<ba_vt::UI32>();
			numProperties = source.read<ba_vt::UI32>();
			propertyListLen = source.read<ba_vt::UI32>();
		} else {
			endOffset = source.read<ba_vt::UI64>();
			numProperties = source.read<ba_vt::UI64>();
			propertyListLen = source.read<ba_vt::UI64>();
		}
		auto nameSize = source.read<ba_vt::UI8>();
		auto name = source.read<ba_vt::STR_V, false>(nameSize);

		if (!endOffset) return true;

		auto pos = source.getPosition();

		Node::Property* properties = nullptr;
		if (numProperties) {
			properties = new Node::Property[numProperties];
			for (size_t i = 0; i < numProperties; ++i) {
				if (!parseNodeProperty(fbx, source, properties[i])) {
					delete[] properties;
					return false;
				}
			}
		}

		auto node = new Node(Node::getType(name), properties, numProperties);
		parent->addChild(node);
		fbx.addNode(node);

		source.setPosition(pos + propertyListLen);

		while (source.getPosition() < endOffset) {
			if (!parseNode(fbx, source, node)) return false;
		}

		return true;
	}

	inline FBXConverter::Result AE_CALL parse(const ByteArray& source) {
		ByteArray src = source.slice();
		src.setEndian(std::endian::little);

		constexpr size_t HEADER_LEN = 12;

		if (src.read<ba_vt::STR_V, false>(HEADER_LEN) == "Kaydara FBX ") {
			constexpr size_t BINARY_FLAG_LEN = 8;
			if (src.read<ba_vt::STR_V, false>(BINARY_FLAG_LEN) == "Binary  ") {
				src.skip(3);

				FBX fbx(src.read<ba_vt::UI32>());

				auto parseOk = true;
				while (src.getBytesAvailable() > 4) {
					if (src.read<ba_vt::UI32>() < src.getLength()) {
						src.setPosition(src.getPosition() - 4);
						if (!parseNode(fbx, src, &fbx.root)) {
							parseOk = false;
							break;
						}
					} else {
						break;
					}
				}

				if (parseOk) {
					FBXConverter::Result rst;
					fbx.build(rst);
					return std::move(rst);
				}
			} else {
				printdln("FBX parse error : only support binary format");
			}
		} else {
			printdln("FBX parse error : not a fbx file");
		}

		return FBXConverter::Result();
	}
}