#pragma once

#include "base/Ref.h"

AE_NODE_NS_BEGIN

class Node;

AE_NODE_NS_END

AE_NODE_COMPONENT_NS_BEGIN

class AE_DLL ComponentFlag {
public:
	ComponentFlag() = delete;
	ComponentFlag(const ComponentFlag&) = delete;
	ComponentFlag(ComponentFlag&&) = delete;

	static const ui32 CAMERA = 0b1;
	static const ui32 RENDERABLE = 0b10;
};


class AE_DLL AbstractComponent : public Ref {
public:
	ui32 flag;
	ui32 layer;

	inline bool AE_CALL getEnalbed() const;
	void AE_CALL setEnabled(bool b);
	inline Node* AE_CALL getNode() const;

	void AE_CALL __setNode(Node* node);

protected:
	bool _enabled;
	Node* _node;

	AbstractComponent();

	virtual void AE_CALL _nodeChanged(Node* old) {};
	virtual void AE_CALL _enabledChanged() {};
};

AE_NODE_COMPONENT_NS_END

#include "AbstractComponent.inl"
