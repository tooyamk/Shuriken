#pragma once

#include "FBXConverter.h"
#include "srk/ByteArray.h"
#include "srk/Printer.h"
#include "srk/Mesh.h"
#include "srk/ShaderPredefine.h"
#include "zlib.h"
#include <vector>
#include <unordered_map>

namespace srk::extensions::fbx_converter {
	using namespace std::literals;

#ifndef SRK_FBX_ENUM
#	define SRK_FBX_ENUM

#	define SRK_FBX_ENUM_Type \
			SRK_FBX_ENUM_ELEMENT(Unknown) \
			SRK_FBX_ENUM_ELEMENT(AnimationCurve) \
			SRK_FBX_ENUM_ELEMENT(AnimationCurveNode) \
			SRK_FBX_ENUM_ELEMENT(AnimationLayer) \
			SRK_FBX_ENUM_ELEMENT(AnimationStack) \
			SRK_FBX_ENUM_ELEMENT(BlendWeights) \
			SRK_FBX_ENUM_ELEMENT(C) \
			SRK_FBX_ENUM_ELEMENT(Connections) \
			SRK_FBX_ENUM_ELEMENT(Deformer) \
			SRK_FBX_ENUM_ELEMENT(Geometry) \
			SRK_FBX_ENUM_ELEMENT(GlobalSettings) \
			SRK_FBX_ENUM_ELEMENT(Indexes) \
			SRK_FBX_ENUM_ELEMENT(KeyTime) \
			SRK_FBX_ENUM_ELEMENT(KeyValueFloat) \
			SRK_FBX_ENUM_ELEMENT(LayerElementBinormal) \
			SRK_FBX_ENUM_ELEMENT(LayerElementNormal) \
			SRK_FBX_ENUM_ELEMENT(LayerElementTangent) \
			SRK_FBX_ENUM_ELEMENT(LayerElementUV) \
			SRK_FBX_ENUM_ELEMENT(MappingInformationType) \
			SRK_FBX_ENUM_ELEMENT(Matrix) \
			SRK_FBX_ENUM_ELEMENT(Model) \
			SRK_FBX_ENUM_ELEMENT(Node) \
			SRK_FBX_ENUM_ELEMENT(Normals) \
			SRK_FBX_ENUM_ELEMENT(Tangents) \
			SRK_FBX_ENUM_ELEMENT(Binormals) \
			SRK_FBX_ENUM_ELEMENT(Objects) \
			SRK_FBX_ENUM_ELEMENT(P) \
			SRK_FBX_ENUM_ELEMENT(PolygonVertexIndex) \
			SRK_FBX_ENUM_ELEMENT(Pose) \
			SRK_FBX_ENUM_ELEMENT(PoseNode) \
			SRK_FBX_ENUM_ELEMENT(Properties70) \
			SRK_FBX_ENUM_ELEMENT(ReferenceInformationType) \
			SRK_FBX_ENUM_ELEMENT(Transform) \
			SRK_FBX_ENUM_ELEMENT(TransformLink) \
			SRK_FBX_ENUM_ELEMENT(UV) \
			SRK_FBX_ENUM_ELEMENT(UVIndex) \
			SRK_FBX_ENUM_ELEMENT(Vertices) \
			SRK_FBX_ENUM_ELEMENT(Weights) \

#	define SRK_FBX_ENUM_AttribType \
			SRK_FBX_ENUM_ELEMENT(Unknown) \
			SRK_FBX_ENUM_ELEMENT(Cluster) \
			SRK_FBX_ENUM_ELEMENT(LimbNode) \
			SRK_FBX_ENUM_ELEMENT(Root) \
			SRK_FBX_ENUM_ELEMENT(Null) \
			SRK_FBX_ENUM_ELEMENT(Mesh) \
			SRK_FBX_ENUM_ELEMENT(Skin) \
			SRK_FBX_ENUM_ELEMENT(T) \
			SRK_FBX_ENUM_ELEMENT(R) \
			SRK_FBX_ENUM_ELEMENT(S) \

#endif

	class Node {
	public:
		enum class Type : uint8_t {
#define SRK_FBX_ENUM_ELEMENT(a) a,
			SRK_FBX_ENUM_Type
#undef SRK_FBX_ENUM_ELEMENT
			__end
		};

		inline static const std::unordered_map<std::string_view, Type> TYPE_MAP = {
#define SRK_FBX_ENUM_ELEMENT(a) { #a##sv, Type::a },
			SRK_FBX_ENUM_Type
#undef SRK_FBX_ENUM_ELEMENT
			{ "__end"sv, Type::Unknown }
		};

		enum class AttribType : uint8_t {
#define SRK_FBX_ENUM_ELEMENT(a) a,
			SRK_FBX_ENUM_AttribType
#undef SRK_FBX_ENUM_ELEMENT
			__end
		};

		inline static const std::unordered_map<std::string_view, AttribType> ATTRIB_TYPE_MAP = {
#define SRK_FBX_ENUM_ELEMENT(a) { #a##sv, AttribType::a },
			SRK_FBX_ENUM_AttribType
#undef SRK_FBX_ENUM_ELEMENT
			{ "__end"sv, AttribType::Unknown }
		};

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

			inline bool SRK_CALL operator!=(const iterator& itr) const {
				return _node != itr._node;
			}

			inline iterator& SRK_CALL operator++() {
				if (_node) _node = _node->_next;
				return *this;
			}

			inline iterator SRK_CALL operator++(int32_t) const {
				return iterator(_node ? _node->_next : nullptr);
			}

			inline Node*& SRK_CALL operator*() {
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
				type(Type::UNKNOWN),
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
					if (auto& p = _properties[2]; p.type == Property::Type::STR) {
						auto itr = ATTRIB_TYPE_MAP.find(std::string_view((char*)p.rawVal.data, p.rawVal.size));
						_attribType = itr != ATTRIB_TYPE_MAP.end() ? itr->second : AttribType::Unknown;
					}
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

		inline iterator SRK_CALL begin() const {
			return iterator(_childHead);
		}
		inline iterator SRK_CALL end() const {
			return iterator();
		}

		inline Type SRK_CALL getType() const {
			return _type;
		}

		inline int64_t SRK_CALL getID() const {
			return _id;
		}

		inline const std::string_view& SRK_CALL getAttribName() const {
			return _attribName;
		}

		inline AttribType SRK_CALL getAttribType() const {
			return _attribType;
		}

		inline uint16_t SRK_CALL numProperties() const {
			return _numProperties;
		}

		inline Property* SRK_CALL getProperties() {
			return _properties;
		}

		inline const Property* SRK_CALL getProperty(Property::Type pt) const {
			for (size_t i = 0; i < _numProperties; ++i) {
				if (_properties[i].type == pt) return &_properties[i];
			}
			return nullptr;
		}

		void SRK_CALL addChild(Node* child) {
			if (_childHead) {
				_childTail->_next = child;
				_childTail = child;
			} else {
				_childHead = child;
				_childTail = child;
			}
		}

		Node* SRK_CALL getChild(Type type) {
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

		void SRK_CALL addNode(Node* node) {
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

		inline Node* SRK_CALL getNode(int64_t id) const {
			auto itr = objects.find(id);
			return itr == objects.end() ? nullptr : itr->second;
		}

		inline std::vector<Connection>* SRK_CALL getConnectionChildren(int64_t id) {
			auto itr = connectionChildren.find(id);
			return itr == connectionChildren.end() ? nullptr : &itr->second;
		}

		inline std::vector<Connection>* SRK_CALL getConnectionParents(int64_t id) {
			auto itr = connectionParents.find(id);
			return itr == connectionParents.end() ? nullptr : &itr->second;
		}

		Node* SRK_CALL findConnectionChildNode(int64_t id, Node::Type type) {
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

		void SRK_CALL build(FBXConverter::Result& rst) {
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
								if (vr.desc.format.dimension == 3) {
									switch (vr.desc.format.type) {
									case modules::graphics::VertexType::I16:
									case modules::graphics::VertexType::UI16:
										_transformXZY<2>(vr.resource->data);
										break;
									case modules::graphics::VertexType::I32:
									case modules::graphics::VertexType::UI32:
									case modules::graphics::VertexType::F32:
										_transformXZY<4>(vr.resource->data);
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

		void SRK_CALL _addConnection(ConnectionType& connections, int64_t key, int64_t id, const std::string_view& relationship) {
			connections.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple()).first->second.emplace_back(id, relationship);
		}

		template<size_t Size>
		void SRK_CALL _transformXZY(ByteArray& data) {
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

		void SRK_CALL _buildGlobalSettings() {
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

		MeshResource* SRK_CALL _buildGeometry(Node& node, SkeletonData& skeleton) {
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

		std::tuple<bool, size_t> SRK_CALL _buildPolygonVertexIndex(Node& node, std::vector<uint32_t>& sourceIndices, std::vector<uint8_t>& faces) {
			auto triangulate = false;
			size_t numTriangulateIndices = 0;

			if (node.numProperties() > 0) {
				if (auto& p = node.getProperties()[0]; p.type == Node::Property::Type::I32_ARR) {
					auto count = p.rawVal.size;
					ByteArray src(p.rawVal.data, count << 2, ByteArray::Usage::SHARED);

					size_t start = 0;
					sourceIndices.resize(count);
					for (size_t i = 0; i < count; ++i) {
						auto idx = src.read<int32_t>();
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

		void SRK_CALL _buildPosition(Node& node, MeshResource& mesh, const std::vector<uint32_t>& sourceIndices, const std::vector<uint8_t>& faces, bool triangulate, size_t numTriangulateIndices) {
			if (node.numProperties() > 0) {
				if (auto& p = node.getProperties()[0]; p.type == Node::Property::Type::F32_ARR || p.type == Node::Property::Type::F64_ARR) {
					auto vs = new VertexResource();
					size_t dim = 3;
					vs->stride = dim * sizeof(float32_t);
					if (p.type == Node::Property::Type::F32_ARR) {
						vs->data = _buildPosition<float32_t, float32_t>(p, sourceIndices);
					} else {
						vs->data = _buildPosition<float64_t, float32_t>(p, sourceIndices);
					}

					mesh.setVertex(ShaderPredefine::POSITION0, modules::graphics::VertexAttribute<VertexResource>(vs, dim, modules::graphics::VertexType::F32, 0));

					if (auto count = sourceIndices.size(); p.rawVal.size <= BitUInt<16>::MAX) {
						_buildIndices<uint16_t>(mesh, faces, count, triangulate, numTriangulateIndices);
					} else {
						_buildIndices<uint32_t>(mesh, faces, count, triangulate, numTriangulateIndices);
					}
				}
			}
		}

		template<typename SrcType, typename DstType>
		ByteArray SRK_CALL _buildPosition(const Node::Property& p, const std::vector<uint32_t>& sourceIndices) {
			auto count = sourceIndices.size();

			if constexpr (std::same_as<SrcType, DstType>) {
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
		void SRK_CALL _buildIndices(MeshResource& mesh, const std::vector<uint8_t>& faces, size_t count, bool triangulate, size_t numTriangulateIndices) {
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
		void SRK_CALL _buildOtherVertices(Node& node, MeshResource& mesh, const std::string& key, const std::vector<uint32_t>& sourceIndices) {
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
				size_t dim = 3;
				if (values->type == Node::Property::Type::F32_ARR) {
					vs = _buildVertexSource<float32_t, uint32_t, float32_t>(*values, nullptr, rt, mt, sourceIndices, dim);
				} else {
					vs = _buildVertexSource<float64_t, uint32_t, float32_t>(*values, nullptr, rt, mt, sourceIndices, dim);
				}
				if (vs) {
					mesh.setVertex(key, modules::graphics::VertexAttribute<VertexResource>(vs, dim, modules::graphics::VertexType::F32, 0));
				}
			}
		}

		void SRK_CALL _buildUVs(Node& node, MeshResource& mesh, const std::vector<uint32_t>& sourceIndices) {
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
				size_t dim = 2;
				if (values->type == Node::Property::Type::F32_ARR) {
					if (indices->type == Node::Property::Type::I32_ARR) {
						vs = _buildVertexSource<float32_t, uint32_t, float32_t>(*values, indices, rt, mt, sourceIndices, dim);
					} else {
						vs = _buildVertexSource<float32_t, uint64_t, float32_t>(*values, indices, rt, mt, sourceIndices, dim);
					}
				} else {
					if (indices->type == Node::Property::Type::I32_ARR) {
						vs = _buildVertexSource<float64_t, uint32_t, float32_t>(*values, indices, rt, mt, sourceIndices, dim);
					} else {
						vs = _buildVertexSource<float64_t, uint64_t, float32_t>(*values, indices, rt, mt, sourceIndices, dim);
					}
				}
				if (vs) {
					mesh.setVertex(ShaderPredefine::UV0, modules::graphics::VertexAttribute<VertexResource>(vs, dim, modules::graphics::VertexType::F32, 0));
				}
			}
		}

		template<typename ValueType, typename IndexType, typename DstType>
		VertexResource* SRK_CALL _buildVertexSource(const Node::Property& valueProperty, const Node::Property* indexProperty, 
			const std::string_view& refType, const std::string_view& mappingType, const std::vector<uint32_t>& sourceIndices, size_t numDataPerVertex) {
			auto size = sourceIndices.size();
			const auto DST_VERTEX_SIZE = sizeof(DstType) * numDataPerVertex;
			auto values = (ValueType*)valueProperty.rawVal.data;
			ByteArray dst(size * DST_VERTEX_SIZE);

			if (mappingType == Node::Property::Value::ByControlVertex) {
				if (refType == Node::Property::Value::Direct) {
					if constexpr (std::same_as<ValueType, DstType>) {
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

					if constexpr (std::same_as<ValueType, DstType>) {
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
					if constexpr (std::same_as<ValueType, DstType>) {
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

					if constexpr (std::same_as<ValueType, DstType>) {
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
			vs->stride = DST_VERTEX_SIZE;

			return vs;
		}

		void SRK_CALL _buildSkin() {

		}
	};

	inline bool SRK_CALL decodeNodeProperty(FBX& fbx, ByteArray& source, Node::Property& p) {
		using namespace std::literals;

		auto type = (Node::Property::Type)source.read<uint8_t>();
		p.type = type;
		switch (type) {
		case Node::Property::Type::BOOL:
			p.boolVal = source.read<bool>();
			break;
		case Node::Property::Type::I16:
		{
			p.type = Node::Property::Type::I64;
			p.i64Val = source.read<int16_t>();
			break;
		}
		case Node::Property::Type::I32:
		{
			p.type = Node::Property::Type::I64;
			p.i64Val = source.read<int32_t>();
			break;
		}
		case Node::Property::Type::I64:
			p.i64Val = source.read<int64_t>();
			break;
		case Node::Property::Type::F32:
			p.f32Val = source.read<float32_t>();
			break;
		case Node::Property::Type::F64:
			p.f64Val = source.read<float64_t>();
			break;
		case Node::Property::Type::BYTES:
		case Node::Property::Type::STR:
		{
			auto size = source.read<uint32_t>();
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
			auto arrSize = source.read<int32_t>();
			auto encoding = source.read<int32_t>();
			auto compressedLength = source.read<int32_t>();

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
							printaln(L"FBX decode error : uncompress error "sv);
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
			printaln(L"FBX decode error : Unknown property type "sv, type);
			return false;
		}

		return true;
	}

	inline bool SRK_CALL decodeNode(FBX& fbx, ByteArray& source, Node* parent) {
		uint64_t endOffset, numProperties, propertyListLen;
		if (fbx.ver < 7500) {
			endOffset = source.read<uint32_t>();
			numProperties = source.read<uint32_t>();
			propertyListLen = source.read<uint32_t>();
		} else {
			endOffset = source.read<uint64_t>();
			numProperties = source.read<uint64_t>();
			propertyListLen = source.read<uint64_t>();
		}
		auto nameSize = source.read<uint8_t>();
		auto name = source.read<std::string_view, false>(nameSize);

		if (!endOffset) return true;

		auto pos = source.getPosition();

		Node::Property* properties = nullptr;
		if (numProperties) {
			properties = new Node::Property[numProperties];
			for (size_t i = 0; i < numProperties; ++i) {
				if (!decodeNodeProperty(fbx, source, properties[i])) {
					delete[] properties;
					return false;
				}
			}
		}

		auto itr = Node::TYPE_MAP.find(name);
		auto node = new Node(itr != Node::TYPE_MAP.end() ? itr->second : Node::Type::Unknown, properties, numProperties);
		parent->addChild(node);
		fbx.addNode(node);

		source.setPosition(pos + propertyListLen);

		while (source.getPosition() < endOffset) {
			if (!decodeNode(fbx, source, node)) return false;
		}

		return true;
	}

	inline FBXConverter::Result SRK_CALL decode(const ByteArray& source) {
		using namespace std::literals;

		ByteArray src = source.slice();
		src.setEndian(std::endian::little);

		constexpr size_t HEADER_LEN = 12;

		if (src.read<std::string_view, false>(HEADER_LEN) == "Kaydara FBX ") {
			constexpr size_t BINARY_FLAG_LEN = 8;
			if (src.read<std::string_view, false>(BINARY_FLAG_LEN) == "Binary  ") {
				src.skip(3);

				FBX fbx(src.read<uint32_t>());

				auto parseOk = true;
				while (src.getBytesAvailable() > 4) {
					if (src.read<uint32_t>() < src.getLength()) {
						src.setPosition(src.getPosition() - 4);
						if (!decodeNode(fbx, src, &fbx.root)) {
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
				printaln(L"FBX decode error : only support binary format"sv);
			}
		} else {
			printaln(L"FBX decode error : not a fbx file"sv);
		}

		return FBXConverter::Result();
	}
}