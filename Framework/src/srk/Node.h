#pragma once

#include "srk/Intrusive.h"
#include "srk/math/Matrix.h"
#include "srk/math/Quaternion.h"
#include "srk/math/Vector.h"
#include "srk/components/IComponent.h"
#include <functional>
#include <vector>

namespace srk::components {
	class IComponent;
}

namespace srk {
	class SRK_FW_DLL Node : public Ref {
	public:
		class SRK_FW_DLL iterator {
		public:
			iterator(Node* node = nullptr) :
				_node(node) {
			}

			inline iterator& SRK_CALL operator=(const iterator& itr) {
				_node = itr._node;
				return *this;
			}

			inline bool SRK_CALL operator==(const iterator& itr) const {
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


		enum class Result : uint8_t {
			SUCCESS,
			NOT_FOUND,
			ALREADY_EXISTS,
			ALREADY_EXISTS_SINGLE,
			NOT_NULL,
			ALREADY_HAS_NODE,
			NODE_NOT_SELF,
			CANNOT_IS_SELF,
			CANNOT_HAS_PARENT,
			CANNOT_IS_ROOT_OF_SELF,
			ISNOT_CHILD_OF_SELF,
			BEFORE_ISNOT_CHILD_OF_SELF
		};


		Node();
		virtual ~Node();

		std::string name;

		inline iterator SRK_CALL begin() const {
			return iterator(_childHead);
		}
		inline iterator SRK_CALL end() const {
			return iterator();
		}

		inline Node* SRK_CALL getRoot() const {
			return _root;
		}
		inline Node* SRK_CALL getParent() const {
			return _parent;
		}
		inline uint32_t SRK_CALL getNumChildren() const {
			return _numChildren;
		}

		template<std::derived_from<Node> T, typename... Args>
		inline T* SRK_CALL addChild(Args&&... args) {
			auto child = new T(std::forward<Args>(args)...);
			_addChild(child);
			return child;
		}
		Result SRK_CALL addChild(Node* child);
		Result SRK_CALL insertChild(Node* child, Node* before);
		Result SRK_CALL removeChild(Node* child);
		iterator SRK_CALL removeChild(const iterator& itr);
		bool SRK_CALL removeFromParent();
		size_t SRK_CALL removeAllChildren();

		inline void SRK_CALL getLocalPosition(float32_t(&dst)[3]) const {
			auto x = _lm.data[0][3], y = _lm.data[1][3], z = _lm.data[2][3];
			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}
		inline Vec3f32 SRK_CALL getLocalPosition() const {
			return Vec3f32(_lm.data[0][3], _lm.data[1][3], _lm.data[2][3]);
		}
		void SRK_CALL setLocalPosition(const float32_t(&p)[3]);
		void SRK_CALL localTranslate(const float32_t(&p)[3]);

		inline const Quaternion<float32_t>& SRK_CALL getLocalRotation() const {
			return _lr;
		}
		void SRK_CALL setLocalRotation(const Quaternion<float32_t>& q);
		void SRK_CALL localRotate(const Quaternion<float32_t>& q);

		inline const Vec3f32& SRK_CALL getLocalScale() const {
			return _ls;
		}
		void SRK_CALL setLocalScale(const float32_t(&s)[3]);

		inline const Matrix3x4f32& SRK_CALL getLocalMatrix() const {
			updateLocalMatrix();

			return _lm;
		}
		void SRK_CALL setLocalMatrix(const Matrix3x4f32& m);
		void SRK_CALL setLocalTRS(const float32_t(&pos)[3], const Quaternion<float32_t>& rot, const float32_t(&scale)[3]);

		void SRK_CALL parentTranslate(const float32_t(&p)[3]);

		void SRK_CALL parentRotate(const Quaternion<float32_t>& q);

		inline void SRK_CALL getWorldPosition(float32_t(&dst)[3]) const {
			updateWorldMatrix();

			auto x = _wm.data[0][3], y = _wm.data[1][3], z = _wm.data[2][3];
			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}
		inline Vec3f32 SRK_CALL getWorldPosition() const {
			updateWorldMatrix();

			return Vec3f32(_wm.data[0][3], _wm.data[1][3], _wm.data[2][3]);
		}
		void SRK_CALL setWorldPosition(const float32_t(&p)[3]);
		void SRK_CALL worldTranslate(const float32_t(&p)[3]);

		inline const Quaternion<float32_t>& SRK_CALL getWorldRotation() const {
			updateWorldRotation();
			return _wr;
		}
		void SRK_CALL setWorldRotation(const Quaternion<float32_t>& q);
		void SRK_CALL worldRotate(const Quaternion<float32_t>& q);

		inline const Matrix3x4f32& SRK_CALL getWorldMatrix() const {
			updateWorldMatrix();

			return _wm;
		}
		void SRK_CALL setWorldMatrix(const Matrix3x4f32& m);

		inline const Matrix3x4f32& SRK_CALL getInverseWorldMatrix() const {
			updateInverseWorldMatrix();

			return _iwm;
		}

		void SRK_CALL setIdentity();

		void SRK_CALL updateLocalMatrix() const;
		void SRK_CALL updateWorldRotation() const;
		void SRK_CALL updateWorldMatrix() const;
		void SRK_CALL updateInverseWorldMatrix() const;

		/**
		 * (node).setLocalRotation(dst)
		 * (node).worldRotation = worldRot
		 *
		 * @param worldRot Target world rotation.
		 */
		static void SRK_CALL getLocalRotationFromWorld(const Node& node, const Quaternion<float32_t>& worldRot, Quaternion<float32_t>& dst);
		inline static Quaternion<float32_t> SRK_CALL getLocalRotationFromWorld(const Node& node, const Quaternion<float32_t>& worldRot) {
			Quaternion<float32_t> q(nullptr);
			getLocalRotationFromWorld(node, worldRot, q);
			return q;
		}

	protected:
		enum class DirtyFlag : uint8_t {
			EMPTY = 0,
			LM = 0b1,
			NOT_LM = (std::underlying_type_t<DirtyFlag>)~LM,
			WM = 0b10,
			NOT_WM = (std::underlying_type_t<DirtyFlag>)~WM,
			IWM = 0b100,
			NOT_IWM = (std::underlying_type_t<DirtyFlag>)~IWM,
			WQ = 0b1000,
			NOT_WQ = (std::underlying_type_t<DirtyFlag>)~WQ,
			WM_IWM = WM | IWM,
			WM_IWM_WQ = WM_IWM | WQ,
			LM_WM_IWM_WQ = LM | WM_IWM_WQ,
			LM_WM_IWM = LM | WM_IWM
		};


		Node* _parent;
		Node* _root;

		Node* _prev;
		Node* _next;

		Node* _childHead;
		uint32_t _numChildren;

		mutable DirtyFlag _dirty;

		mutable Quaternion<float32_t> _lr;
		mutable Vec3f32 _ls;
		mutable Matrix3x4f32 _lm;

		mutable Quaternion<float32_t> _wr;
		mutable Matrix3x4f32 _wm;
		mutable Matrix3x4f32 _iwm;

		inline void SRK_CALL _addChild(Node* child) {
			child->ref();
			_addNode(child);
			child->_parentChanged(_root);
		}

		void SRK_CALL _addNode(Node* child);
		void SRK_CALL _insertNode(Node* child, Node* before);
		void SRK_CALL _removeNode(Node* child);

		inline void SRK_CALL _parentChanged(Node* root) {
			_root = root;

			_checkNoticeUpdate(DirtyFlag::WM_IWM_WQ);
		}

		inline void SRK_CALL _localDecomposition() {
			Matrix3x3f32 rot(nullptr);
			_lm.decompose(rot.data, _ls.data);
			rot.toQuaternion(_lr.data);
		}
		void SRK_CALL _worldPositionChanged(DirtyFlag oldDirty);
		void SRK_CALL _worldRotationChanged(DirtyFlag oldDirty);
		inline void SRK_CALL _checkNoticeUpdate(DirtyFlag dirty) {
			using namespace srk::enum_operators;

			_checkNoticeUpdateNow(_dirty | dirty, dirty);
		}
		inline void SRK_CALL _checkNoticeUpdate(DirtyFlag appendDirty, DirtyFlag sendDirty) {
			using namespace srk::enum_operators;

			_checkNoticeUpdateNow(_dirty | appendDirty, sendDirty);
		}
		inline void SRK_CALL _checkNoticeUpdateNow(DirtyFlag nowDirty, DirtyFlag sendDirty) {
			if (nowDirty != _dirty) {
				_dirty = nowDirty;

				_noticeUpdate(sendDirty);
			}
		}
		void SRK_CALL _noticeUpdate(DirtyFlag dirty);
	};
}