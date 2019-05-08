#pragma once

#include "base/Ref.h"
#include "math/Matrix34.h"
#include "math/Quaternion.h"
#include "math/Vector.h"
#include <functional>
#include <vector>

namespace aurora::nodes::component {
	class AbstractComponent;
}

namespace aurora::nodes {
	class AE_DLL Node : public Ref {
	public:
		Node();
		virtual ~Node();

		std::string name;

		inline Node* AE_CALL getRoot() const;
		inline Node* AE_CALL getParent() const;
		inline ui32 AE_CALL getNumChildren() const;

		Node* AE_CALL addChild(Node* child);
		Node* AE_CALL insertChild(Node* child, Node* before);
		bool AE_CALL removeChild(Node* child);
		bool AE_CALL removeFromParent();
		void AE_CALL removeAllChildren();

		inline void AE_CALL getLocalPosition(f32(&dst)[3]) const;
		void AE_CALL setLocalPosition(const f32(&p)[3]);
		void AE_CALL localTranslate(const f32(&p)[3]);

		inline const Quaternion& AE_CALL getLocalRotation() const;
		void AE_CALL setLocalRotation(const Quaternion& q);
		void AE_CALL localRotate(const Quaternion& q);

		inline const Vec3f32& AE_CALL getLocalScale() const;
		void AE_CALL setLocalScale(const f32(&s)[3]);

		inline const Matrix34& AE_CALL getLocalMatrix() const;
		void AE_CALL setLocalMatrix(const Matrix34& m);
		void AE_CALL setLocalTRS(const f32(&pos)[3], const Quaternion& rot, const f32(&scale)[3]);

		void AE_CALL parentRotate(const Quaternion& q);

		inline void AE_CALL getWorldPosition(f32(&dst)[3]) const;
		void AE_CALL setWorldPosition(const f32(&p)[3]);
		void AE_CALL worldTranslate(const f32(&p)[3]);

		inline const Quaternion& AE_CALL getWorldRotation() const;
		void AE_CALL setWorldRotation(const Quaternion& q);
		void AE_CALL worldRotate(const Quaternion& q);

		inline const Matrix34& AE_CALL getWorldMatrix() const;
		void AE_CALL setWorldMatrix(const Matrix34& m);

		inline const Matrix34& AE_CALL getInverseWorldMatrix() const;

		void AE_CALL setIdentity();

		void AE_CALL updateLocalMatrix() const;
		void AE_CALL updateWorldRotation() const;
		void AE_CALL updateWorldMatrix() const;
		void AE_CALL updateInverseWorldMatrix() const;

		void AE_CALL addComponent(component::AbstractComponent* component);
		void AE_CALL removeComponent(component::AbstractComponent* component);
		void AE_CALL removeAllComponents();
		component::AbstractComponent* AE_CALL getComponent(ui32 flag) const;
		component::AbstractComponent* AE_CALL getComponentIf(const std::function<bool(component::AbstractComponent*)>& func) const;
		inline const std::vector<component::AbstractComponent*>& AE_CALL getComponents() const;
		void AE_CALL getComponents(ui32 flag, std::vector<component::AbstractComponent*>& dst) const;
		void AE_CALL getComponentsIf(const std::function<bool(component::AbstractComponent*)>& func, std::vector<component::AbstractComponent*>& dst) const;

		/**
		 * (node).setLocalRotation(dst)
		 * (node).worldRotation = worldRot
		 *
		 * @param worldRot Target world rotation.
		 */
		static void AE_CALL getLocalRotationFromWorld(const Node& node, const Quaternion& worldRot, Quaternion& dst);

	protected:
		struct DirtyFlag {
			static const ui32 LM = 0b1;
			static const ui32 NOT_LM = ~LM;
			static const ui32 WM = 0b10;
			static const ui32 NOT_WM = ~WM;
			static const ui32 WIM = 0b100;
			static const ui32 NOT_WIM = ~WIM;
			static const ui32 WR = 0b1000;
			static const ui32 NOT_WR = ~WR;
			static const ui32 WMIM = WM | WIM;
			static const ui32 WRMIM = WMIM | WR;
			static const ui32 LM_WRMIM = LM | WRMIM;
			static const ui32 LM_WMIM = LM | WMIM;
		};


		Node* _parent;
		Node* _root;

		Node* _prev;
		Node* _next;

		Node* _childHead;
		ui32 _numChildren;

		mutable Quaternion _lr;
		mutable Vec3f32 _ls;
		mutable Matrix34 _lm;

		mutable Quaternion _wr;
		mutable Matrix34 _wm;
		mutable Matrix34 _iwm;

		mutable ui32 _dirty;
		std::vector<component::AbstractComponent*> _components;

		void AE_CALL _addNode(Node* child);
		void AE_CALL _insertNode(Node* child, Node* before);
		void AE_CALL _removeNode(Node* child);

		void AE_CALL _addChild(Node* child);
		void AE_CALL _parentChanged(Node* root);

		inline void AE_CALL _localDecomposition();
		void AE_CALL _worldPositionChanged(ui32 oldDirty);
		void AE_CALL _worldRotationChanged(ui32 oldDirty);
		inline void AE_CALL _checkNoticeUpdate(ui32 dirty);
		inline void AE_CALL _checkNoticeUpdate(ui32 appendDirty, ui32 sendDirty);
		void AE_CALL _checkNoticeUpdateNow(ui32 nowDirty, ui32 sendDirty);
		void AE_CALL _noticeUpdate(ui32 dirty);

		void AE_CALL _removeComponent(component::AbstractComponent* component);
	};
}

#include "Node.inl"