#pragma once

#include "aurora/Ref.h"
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
	class AE_DLL Node : public Ref {
	public:
		class AE_DLL iterator {
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

		template<typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<Node, T>, T>>
		inline T* AE_CALL addChild(Args... args) {
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

		Result AE_CALL addComponent(components::IComponent* component);

		template<typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<components::IComponent, T>, T>>
		std::tuple<Node::Result, T*> AE_CALL addComponent(Args... args) {
			if (auto& ci = T::rttiClassInfo; ci.getSingleBase()) {
				for (auto c : _components) {
					if (ci.getSingleBase() == c->getRttiClassInfo().getSingleBase()) return std::make_tuple(Result::ALREADY_EXISTS_SINGLE, nullptr);
				}
			}

			auto component = new T(std::forward<Args>(args)...);
			_addComponent(component);
			return std::make_tuple(Result::SUCCESS, component);
		}

		Result AE_CALL removeComponent(components::IComponent* component);
		size_t AE_CALL removeAllComponents();

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<components::IComponent, T>, T>>
		inline T* AE_CALL getComponent() const {
			for (auto c : _components) {
				if (c->isKindOf<T>()) return (T*)c;
			}
			return nullptr;
		}
		template<typename Com, typename Fn, typename = std::enable_if_t<std::is_base_of_v<components::IComponent, Com> && std::is_invocable_v<Fn, Com*>, Com>>
		inline void AE_CALL getComponents(const Fn& fn) const {
			for (auto c : _components) {
				if constexpr (std::is_same_v<components::IComponent, Com>) {
					fn((Com*)c);
				} else {
					if (c->isKindOf<Com>()) fn((Com*)c);
				}
			}
		}

		/**
		 * (node).setLocalRotation(dst)
		 * (node).worldRotation = worldRot
		 *
		 * @param worldRot Target world rotation.
		 */
		static void AE_CALL getLocalRotationFromWorld(const Node& node, const Quaternion& worldRot, Quaternion& dst);
		inline static Quaternion AE_CALL getLocalRotationFromWorld(const Node& node, const Quaternion& worldRot) {
			Quaternion q(NO_INIT);
			getLocalRotationFromWorld(node, worldRot, q);
			return q;
		}

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType LM = 0b1;
			static const DirtyType NOT_LM = ~LM;
			static const DirtyType WM = 0b10;
			static const DirtyType NOT_WM = ~WM;
			static const DirtyType WIM = 0b100;
			static const DirtyType NOT_WIM = ~WIM;
			static const DirtyType WR = 0b1000;
			static const DirtyType NOT_WR = ~WR;
			static const DirtyType WMIM = WM | WIM;
			static const DirtyType WRMIM = WMIM | WR;
			static const DirtyType LM_WRMIM = LM | WRMIM;
			static const DirtyType LM_WMIM = LM | WMIM;
		};


		Node* _parent;
		Node* _root;

		Node* _prev;
		Node* _next;

		Node* _childHead;
		uint32_t _numChildren;

		mutable DirtyType _dirty;

		mutable Quaternion _lr;
		mutable Vec3f32 _ls;
		mutable Matrix34 _lm;

		mutable Quaternion _wr;
		mutable Matrix34 _wm;
		mutable Matrix34 _iwm;

		std::vector<components::IComponent*> _components;

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
			Matrix34 rot(NO_INIT);
			_lm.decomposition(&rot, _ls);
			rot.toQuaternion(_lr);
		}
		void AE_CALL _worldPositionChanged(DirtyType oldDirty);
		void AE_CALL _worldRotationChanged(DirtyType oldDirty);
		inline void AE_CALL _checkNoticeUpdate(DirtyType dirty) {
			_checkNoticeUpdateNow(_dirty | dirty, dirty);
		}
		inline void AE_CALL _checkNoticeUpdate(DirtyType appendDirty, DirtyType sendDirty) {
			_checkNoticeUpdateNow(_dirty | appendDirty, sendDirty);
		}
		inline void AE_CALL _checkNoticeUpdateNow(DirtyType nowDirty, DirtyType sendDirty) {
			if (nowDirty != _dirty) {
				_dirty = nowDirty;

				_noticeUpdate(sendDirty);
			}
		}
		void AE_CALL _noticeUpdate(DirtyType dirty);

		inline void AE_CALL _addComponent(components::IComponent* component) {
			component->ref();
			_components.emplace_back(component);
			component->__setNode(this);
		}

		void AE_CALL _removeComponent(components::IComponent* component);
	};
}