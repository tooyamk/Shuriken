#pragma once

#include "aurora/Intrusive.h"
#include "aurora/math/Matrix.h"
#include "aurora/math/Quaternion.h"
#include "aurora/math/Vector.h"
#include "aurora/components/IComponent.h"
#include <functional>
#include <vector>

namespace aurora::components {
	class IComponent;
}

namespace aurora {
	class AE_FW_DLL Node : public Ref {
	public:
		class AE_FW_DLL iterator {
		public:
			iterator(Node* node = nullptr) :
				_node(node) {
			}

			inline iterator& AE_CALL operator=(const iterator& itr) {
				_node = itr._node;
				return *this;
			}

			inline bool AE_CALL operator==(const iterator& itr) const {
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

		inline iterator AE_CALL begin() const {
			return iterator(_childHead);
		}
		inline iterator AE_CALL end() const {
			return iterator();
		}

		inline Node* AE_CALL getRoot() const {
			return _root;
		}
		inline Node* AE_CALL getParent() const {
			return _parent;
		}
		inline uint32_t AE_CALL getNumChildren() const {
			return _numChildren;
		}

		template<std::derived_from<Node> T, typename... Args>
		inline T* AE_CALL addChild(Args&&... args) {
			auto child = new T(std::forward<Args>(args)...);
			_addChild(child);
			return child;
		}
		Result AE_CALL addChild(Node* child);
		Result AE_CALL insertChild(Node* child, Node* before);
		Result AE_CALL removeChild(Node* child);
		iterator AE_CALL removeChild(const iterator& itr);
		bool AE_CALL removeFromParent();
		size_t AE_CALL removeAllChildren();

		inline void AE_CALL getLocalPosition(float32_t(&dst)[3]) const {
			auto x = _lm.data[0][3], y = _lm.data[1][3], z = _lm.data[2][3];
			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}
		inline Vec3f32 AE_CALL getLocalPosition() const {
			return Vec3f32(_lm.data[0][3], _lm.data[1][3], _lm.data[2][3]);
		}
		void AE_CALL setLocalPosition(const float32_t(&p)[3]);
		void AE_CALL localTranslate(const float32_t(&p)[3]);

		inline const Quaternion& AE_CALL getLocalRotation() const {
			return _lr;
		}
		void AE_CALL setLocalRotation(const Quaternion& q);
		void AE_CALL localRotate(const Quaternion& q);

		inline const Vec3f32& AE_CALL getLocalScale() const {
			return _ls;
		}
		void AE_CALL setLocalScale(const float32_t(&s)[3]);

		inline const Matrix34& AE_CALL getLocalMatrix() const {
			updateLocalMatrix();

			return _lm;
		}
		void AE_CALL setLocalMatrix(const Matrix34& m);
		void AE_CALL setLocalTRS(const float32_t(&pos)[3], const Quaternion& rot, const float32_t(&scale)[3]);

		void AE_CALL parentTranslate(const float32_t(&p)[3]);

		void AE_CALL parentRotate(const Quaternion& q);

		inline void AE_CALL getWorldPosition(float32_t(&dst)[3]) const {
			updateWorldMatrix();

			auto x = _wm.data[0][3], y = _wm.data[1][3], z = _wm.data[2][3];
			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}
		inline Vec3f32 AE_CALL getWorldPosition() const {
			updateWorldMatrix();

			return Vec3f32(_wm.data[0][3], _wm.data[1][3], _wm.data[2][3]);
		}
		void AE_CALL setWorldPosition(const float32_t(&p)[3]);
		void AE_CALL worldTranslate(const float32_t(&p)[3]);

		inline const Quaternion& AE_CALL getWorldRotation() const {
			updateWorldRotation();
			return _wr;
		}
		void AE_CALL setWorldRotation(const Quaternion& q);
		void AE_CALL worldRotate(const Quaternion& q);

		inline const Matrix34& AE_CALL getWorldMatrix() const {
			updateWorldMatrix();

			return _wm;
		}
		void AE_CALL setWorldMatrix(const Matrix34& m);

		inline const Matrix34& AE_CALL getInverseWorldMatrix() const {
			updateInverseWorldMatrix();

			return _iwm;
		}

		void AE_CALL setIdentity();

		void AE_CALL updateLocalMatrix() const;
		void AE_CALL updateWorldRotation() const;
		void AE_CALL updateWorldMatrix() const;
		void AE_CALL updateInverseWorldMatrix() const;

		/**
		 * (node).setLocalRotation(dst)
		 * (node).worldRotation = worldRot
		 *
		 * @param worldRot Target world rotation.
		 */
		static void AE_CALL getLocalRotationFromWorld(const Node& node, const Quaternion& worldRot, Quaternion& dst);
		inline static Quaternion AE_CALL getLocalRotationFromWorld(const Node& node, const Quaternion& worldRot) {
			Quaternion q(no_init_v);
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
			WIM = 0b100,
			NOT_WIM = (std::underlying_type_t<DirtyFlag>)~WIM,
			WR = 0b1000,
			NOT_WR = (std::underlying_type_t<DirtyFlag>)~WR,
			WMIM = WM | WIM,
			WRMIM = WMIM | WR,
			LM_WRMIM = LM | WRMIM,
			LM_WMIM = LM | WMIM
		};


		Node* _parent;
		Node* _root;

		Node* _prev;
		Node* _next;

		Node* _childHead;
		uint32_t _numChildren;

		mutable DirtyFlag _dirty;

		mutable Quaternion _lr;
		mutable Vec3f32 _ls;
		mutable Matrix34 _lm;

		mutable Quaternion _wr;
		mutable Matrix34 _wm;
		mutable Matrix34 _iwm;

		inline void AE_CALL _addChild(Node* child) {
			child->ref();
			_addNode(child);
			child->_parentChanged(_root);
		}

		void AE_CALL _addNode(Node* child);
		void AE_CALL _insertNode(Node* child, Node* before);
		void AE_CALL _removeNode(Node* child);

		inline void AE_CALL _parentChanged(Node* root) {
			_root = root;

			_checkNoticeUpdate(DirtyFlag::WRMIM);
		}

		inline void AE_CALL _localDecomposition() {
			Matrix34 rot(no_init_v);
			_lm.decomposition(&rot, _ls);
			rot.toQuaternion(_lr);
		}
		void AE_CALL _worldPositionChanged(DirtyFlag oldDirty);
		void AE_CALL _worldRotationChanged(DirtyFlag oldDirty);
		inline void AE_CALL _checkNoticeUpdate(DirtyFlag dirty) {
			using namespace aurora::enum_operators;

			_checkNoticeUpdateNow(_dirty | dirty, dirty);
		}
		inline void AE_CALL _checkNoticeUpdate(DirtyFlag appendDirty, DirtyFlag sendDirty) {
			using namespace aurora::enum_operators;

			_checkNoticeUpdateNow(_dirty | appendDirty, sendDirty);
		}
		inline void AE_CALL _checkNoticeUpdateNow(DirtyFlag nowDirty, DirtyFlag sendDirty) {
			if (nowDirty != _dirty) {
				_dirty = nowDirty;

				_noticeUpdate(sendDirty);
			}
		}
		void AE_CALL _noticeUpdate(DirtyFlag dirty);
	};
}