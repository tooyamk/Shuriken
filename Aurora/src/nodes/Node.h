#pragma once

#include <functional>
#include <string>
#include <vector>
#include "base/Ref.h"
#include "math/Matrix34.h"
#include "math/Quaternion.h"
#include "math/Vector3.h"

AE_NODE_COMPONENT_NS_BEGIN

class AbstractComponent;

AE_NODE_COMPONENT_NS_END

AE_NODE_NS_BEGIN

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

	inline Vector3 AE_CALL getLocalPosition() const;
	inline void AE_CALL getLocalPosition(Vector3& dst) const;
	void AE_CALL setLocalPosition(const Vector3& p);
	void AE_CALL localTranslate(const Vector3& p);

	inline const Quaternion& AE_CALL getLocalRotation() const;
	void AE_CALL setLocalRotation(const Quaternion& q);
	void AE_CALL localRotate(const Quaternion& q);

	inline const Vector3& AE_CALL getLocalScale() const;
	void AE_CALL setLocalScale(const Vector3& s);

	inline const Matrix34& AE_CALL getLocalMatrix() const;
	void AE_CALL setLocalMatrix(const Matrix34& m);
	void AE_CALL setLocalTRS(const Vector3& pos, const Quaternion& rot, const Vector3& scale);

	void AE_CALL parentRotate(const Quaternion& q);

	inline Vector3 AE_CALL getWorldPosition() const;
	inline void AE_CALL getWorldPosition(Vector3& dst) const;
	void AE_CALL setWorldPosition(const Vector3& p);
	void AE_CALL worldTranslate(const Vector3& p);

	inline const Quaternion& AE_CALL getWorldRotation() const;
	void AE_CALL setWorldRotation(const Quaternion& q);
	void AE_CALL worldRotate(const Quaternion& q);

	inline const Matrix34& AE_CALL getWorldMatrix() const;
	void AE_CALL setWorldMatrix(const Matrix34& m);

	inline const Matrix34& AE_CALL getInverseWorldMatrix() const;

	void AE_CALL setIdentity();

	void AE_CALL updateLocalMatrix();
	void AE_CALL updateWorldRotation();
	void AE_CALL updateWorldMatrix();
	void AE_CALL updateInverseWorldMatrix();

	void AE_CALL addComponent(AE_NODE_COMPONENT_NS::AbstractComponent* component);
	void AE_CALL removeComponent(AE_NODE_COMPONENT_NS::AbstractComponent* component);
	void AE_CALL removeAllComponents();
	AE_NODE_COMPONENT_NS::AbstractComponent* AE_CALL getComponent(ui32 flag) const;
	AE_NODE_COMPONENT_NS::AbstractComponent* AE_CALL getComponentIf(const std::function<bool(AE_NODE_COMPONENT_NS::AbstractComponent*)>& func) const;
	inline const std::vector<AE_NODE_COMPONENT_NS::AbstractComponent*>& AE_CALL getComponents() const;
	void AE_CALL getComponents(ui32 flag, std::vector<AE_NODE_COMPONENT_NS::AbstractComponent*>& dst) const;
	void AE_CALL getComponentsIf(const std::function<bool(AE_NODE_COMPONENT_NS::AbstractComponent*)>& func, std::vector<AE_NODE_COMPONENT_NS::AbstractComponent*>& dst) const;

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

	Quaternion _lr;
	Vector3 _ls;
	Matrix34 _lm;

	Quaternion _wr;
	Matrix34 _wm;
	Matrix34 _iwm;

	ui32 _dirty;
	std::vector<AE_NODE_COMPONENT_NS::AbstractComponent*> _components;

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

	void AE_CALL _removeComponent(AE_NODE_COMPONENT_NS::AbstractComponent* component);
};

AE_NODE_NS_END

#include "Node.inl"