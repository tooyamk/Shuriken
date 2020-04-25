#include "Node.h"
#include "aurora/components/IComponent.h"

namespace aurora {
	Node::Node() :
		name(),
		_parent(nullptr),
		_root(this),
		_prev(nullptr),
		_next(nullptr),
		_childHead(nullptr),
		_numChildren(0),
		_lr(),
		_ls(1.f),
		_lm(),
		_wr(),
		_wm(),
		_iwm(),
		_dirty(0),
		_components() {
	}

	Node::~Node() {
		removeAllChildren();
		removeAllComponents();
	}

	Node* Node::addChild(Node* child) {
		if (child && !child->_parent && child != _root) {
			child->ref();
			_addNode(child);
			child->_parentChanged(_root);

			return child;
		}

		return nullptr;
	}

	Node* Node::insertChild(Node* child, Node* before) {
		if (child && child != _root) {
			if (before) {
				if (before->_parent == this) {
					if (child->_parent == this) {
						if (child == before || child->_prev == before) return child;

						_removeNode(child);
						_insertNode(child, before);

						return child;
					} else if (!child->_parent) {
						child->ref();
						_insertNode(child, before);

						return child;
					}
				}
			} else {
				if (child->_parent == this) {
					_removeNode(child);
					_addNode(child);

					return child;
				} else if (!child->_parent) {
					child->ref();
					_addNode(child);
					child->_parentChanged(_root);

					return child;
				}
			}
		}

		return nullptr;
	}

	bool Node::removeChild(Node* child) {
		if (child && child->_parent == this) {
			_removeNode(child);
			child->_parentChanged(child);
			child->unref();

			return true;
		}

		return false;
	}

	Node::iterator Node::removeChild(const iterator& itr) {
		if (auto child = itr._node; child && child->_parent == this) {
			auto next = child->_next;

			_removeNode(child);
			child->_parentChanged(child);
			child->unref();

			return iterator(next);
		}

		return iterator();
	}

	bool Node::removeFromParent() {
		return _parent ? _parent->removeChild(this) : false;
	}

	void Node::removeAllChildren() {
		if (_childHead) {
			auto child = _childHead;
			do {
				auto next = child->_next;

				child->_prev = nullptr;
				child->_next = nullptr;
				child->_parent = nullptr;
				child->_parentChanged(child);
				child->unref();

				child = next;
			} while (child);

			_childHead = nullptr;
			_numChildren = 0;
		}
	}

	void Node::setLocalPosition(const float32_t(&p)[3]) {
		_lm.setPosition(p);

		_checkNoticeUpdate(DirtyFlag::WMIM);
	}

	void Node::localTranslate(const float32_t(&p)[3]) {
		updateLocalMatrix();
		_lm.prependTranslate(p);

		_checkNoticeUpdate(DirtyFlag::WMIM);
	}

	void Node::setLocalRotation(const Quaternion& q) {
		_lr.set(q);

		_checkNoticeUpdate(DirtyFlag::LM_WRMIM, DirtyFlag::WRMIM);
	}

	void Node::localRotate(const Quaternion& q) {
		q.append(_lr, _lr);

		_checkNoticeUpdate(DirtyFlag::LM_WRMIM, DirtyFlag::WRMIM);
	}

	void Node::setLocalScale(const float32_t(&s)[3]) {
		_ls.set(s);

		_checkNoticeUpdate(DirtyFlag::LM_WMIM, DirtyFlag::WMIM);
	}

	void Node::setLocalMatrix(const Matrix34& m) {
		_lm.set34(m);
		_localDecomposition();

		_checkNoticeUpdateNow((_dirty & DirtyFlag::NOT_LM) | DirtyFlag::WRMIM, DirtyFlag::WRMIM);
	}

	void Node::setLocalTRS(const float32_t(&pos)[3], const Quaternion& rot, const float32_t(&scale)[3]) {
		_lm.setPosition(pos);
		_lr.set(rot);
		_ls.set(scale);

		_checkNoticeUpdate(DirtyFlag::LM_WRMIM, DirtyFlag::WRMIM);
	}

	void Node::parentTranslate(const float32_t(&p)[3]) {
		auto& data = _lm.data;
		for (size_t i = 0; i < 3; ++i) data[i][3] += p[i];

		_checkNoticeUpdate(DirtyFlag::WMIM);
	}

	void Node::parentRotate(const Quaternion& q) {
		_lr.append(q);

		_checkNoticeUpdate(DirtyFlag::LM_WRMIM, DirtyFlag::WRMIM);
	}

	void Node::setWorldPosition(const float32_t(&p)[3]) {
		auto old = _dirty;
		updateWorldMatrix();
		_wm.setPosition(p);

		_worldPositionChanged(old);
	}

	void Node::worldTranslate(const float32_t(&p)[3]) {
		auto old = _dirty;
		updateWorldMatrix();
		_wm.prependTranslate(p);

		_worldPositionChanged(old);
	}

	void Node::setWorldRotation(const Quaternion& q) {
		_wr.set(q);

		_worldRotationChanged(_dirty);
	}

	void Node::worldRotate(const Quaternion& q) {
		auto old = _dirty;
		updateWorldRotation();
		q.append(_wr, _wr);

		_worldRotationChanged(old);
	}

	void Node::setWorldMatrix(const Matrix34& m) {
		_wm.set34(m);

		auto now = (_dirty & DirtyFlag::NOT_WM) | DirtyFlag::WIM;

		if (_parent) {
			_wm.append(getInverseWorldMatrix(), _lm);
		} else {
			_lm.set34(_wm);
		}

		_localDecomposition();

		_checkNoticeUpdateNow((now & DirtyFlag::NOT_LM) | DirtyFlag::WR, DirtyFlag::WRMIM);
	}

	void Node::setIdentity() {
		if (!_lr.isIdentity() || !_ls.isEqual(1.f) || _lm.data[0][3] != 0.f || _lm.data[1][3] != 0.f || _lm.data[2][3] != 0.f) {
			_lm.set34();
			_lr.set();
			_ls.set(1.f);

			_checkNoticeUpdate(DirtyFlag::LM_WRMIM, DirtyFlag::WRMIM);
		}
	}

	void Node::updateLocalMatrix() const {
		if (_dirty & DirtyFlag::LM) {
			_dirty &= DirtyFlag::NOT_LM;

			_lr.toMatrix(_lm);
			_lm.prependScale(_ls);
		}
	}

	void Node::updateWorldRotation() const {
		if (_dirty & DirtyFlag::WR) {
			_dirty &= DirtyFlag::NOT_WR;

			auto p = _parent;
			if (p) {
				p->updateWorldRotation();
				_lr.append(_wr, _wr);
			} else {
				_wr.set(_lr);
			}
		}
	}

	void Node::updateWorldMatrix() const {
		if (_dirty & DirtyFlag::WM) {
			_dirty &= DirtyFlag::NOT_WM;

			updateLocalMatrix();
			
			if (_parent) {
				_parent->updateWorldMatrix();
				_lm.append(_parent->_wm, _wm);
			} else {
				_wm.set34(_lm);
			}
		}
	}

	void Node::updateInverseWorldMatrix() const {
		if (_dirty & DirtyFlag::WIM) {
			_dirty &= DirtyFlag::NOT_WIM;

			updateWorldMatrix();
			_wm.invert(_iwm);
		}
	}

	bool Node::addComponent(components::IComponent* component) {
		if (component && !component->getNode()) {
			component->ref();
			_components.emplace_back(component);
			component->__setNode(this);
			return true;
		}
		return false;
	}

	bool Node::removeComponent(components::IComponent* component) {
		if (component && component->getNode() == this) {
			component->__setNode(nullptr);
			_removeComponent(component);
			return true;
		}
		return false;
	}

	void Node::removeAllComponents() {
		for (auto c : _components) {
			c->__setNode(nullptr);
			c->unref();
		}
		_components.clear();
	}

	void Node::getLocalRotationFromWorld(const Node& node, const Quaternion& worldRot, Quaternion& dst) {
		if (node._parent) {
			auto& q = node._parent->getWorldRotation();
			q.invert(dst);
			worldRot.append(dst, dst);
		} else {
			dst.set(worldRot);
		}
	}

	void Node::_addNode(Node* child) {
		if (_childHead) {
			auto tail = _childHead->_prev;

			tail->_next = child;
			child->_prev = tail;
			_childHead->_prev = child;
		} else {
			_childHead = child;
			child->_prev = child;
		}

		child->_parent = this;
		++_numChildren;
	}

	void Node::_insertNode(Node* child, Node* before) {
		child->_next = before;
		child->_prev = before->_prev;
		if (before == _childHead) {
			_childHead = child;
		} else {
			before->_prev->_next = child;
		}
		before->_prev = child;

		child->_parent = this;
		++_numChildren;
	}

	void Node::_removeNode(Node* child) {
		auto next = child->_next;

		if (_childHead == child) {
			_childHead = next;

			if (next) next->_prev = child->_prev;
		} else {
			auto prev = child->_prev;

			prev->_next = next;
			if (next) {
				next->_prev = prev;
			} else {
				_childHead->_prev = prev;
			}
		}

		child->_prev = nullptr;
		child->_next = nullptr;
		child->_parent = nullptr;

		--_numChildren;
	}

	void Node::_addChild(Node* child) {
		if (_childHead) {
			auto tail = _childHead->_prev;

			tail->_next = child;
			child->_prev = tail;
			_childHead->_prev = child;
		} else {
			_childHead = child;
			child->_prev = child;
		}

		child->_parent = this;
		++_numChildren;
	}

	void Node::_worldPositionChanged(DirtyType oldDirty) {
		if (_parent) {
			_parent->updateInverseWorldMatrix();

			float32_t tmp[3] = { _wm.data[0][3], _wm.data[1][3], _wm.data[2][3] };
			Math::matTransformPoint(_parent->_iwm, tmp, tmp);

			_lm.setPosition(tmp);
		} else {
			_lm.setPosition(_wm);
		}

		_dirty |= DirtyFlag::WIM;
		if (oldDirty != _dirty) _noticeUpdate(DirtyFlag::WMIM);
	}

	void Node::_worldRotationChanged(DirtyType oldDirty) {
		if (_parent) {
			_parent->updateWorldRotation();
			_parent->_wr.invert(_lr);
			_wr.append(_lr, _lr);
		} else {
			_lr.set(_wr);
		}

		_dirty &= DirtyFlag::NOT_WR;
		_dirty |= DirtyFlag::LM_WMIM;
		if (oldDirty != _dirty) _noticeUpdate(DirtyFlag::WRMIM);
	}

	void Node::_noticeUpdate(DirtyType dirty) {
		auto node = _childHead;
		while (node) {
			node->_checkNoticeUpdate(dirty);
			node = node->_next;
		}
	}

	void Node::_removeComponent(components::IComponent* component) {
		if (auto itr = std::find(_components.begin(), _components.end(), component); itr != _components.end()) _components.erase(itr);
		component->unref();
	}
}