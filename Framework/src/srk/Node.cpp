#include "Node.h"
#include "srk/components/IComponent.h"

namespace srk {
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
		_dirty(DirtyFlag::EMPTY) {
	}

	Node::~Node() {
		removeAllChildren();
	}

	Node::Result Node::addChild(Node* child) {
		if (!child) return Result::NOT_NULL;
		if (child->_parent) return Result::CANNOT_HAS_PARENT;
		if (child == _root) return Result::CANNOT_IS_ROOT_OF_SELF;

		_addChild(child);

		return Result::SUCCESS;
	}

	Node::Result Node::insertChild(Node* child, Node* before) {
		if (!child) return Result::NOT_NULL;

		if (before) {
			if (before->_parent != this) return Result::BEFORE_ISNOT_CHILD_OF_SELF;

			if (child->_parent == this) {
				if (child == before || child->_prev == before) return Result::SUCCESS;

				_removeNode(child);
				_insertNode(child, before);

				return Result::SUCCESS;
			} else if (!child->_parent) {
				child->ref();
				_insertNode(child, before);

				return Result::SUCCESS;
			}

			return Result::ISNOT_CHILD_OF_SELF;
		} else {
			if (child->_parent == this) {
				_removeNode(child);
				_addNode(child);

				return Result::SUCCESS;
			} else if (!child->_parent) {
				child->ref();
				_addNode(child);
				child->_parentChanged(_root);

				return Result::SUCCESS;
			}

			return Result::ISNOT_CHILD_OF_SELF;
		}
	}

	Node::Result Node::removeChild(Node* child) {
		if (!child) return Result::NOT_NULL;
		if (child->_parent != this) return Result::ISNOT_CHILD_OF_SELF;

		_removeNode(child);
		child->_parentChanged(child);
		Ref::unref(*child);

		return Result::SUCCESS;
	}

	Node::iterator Node::removeChild(const iterator& itr) {
		if (auto child = itr._node; child && child->_parent == this) {
			auto next = child->_next;

			_removeNode(child);
			child->_parentChanged(child);
			Ref::unref(*child);

			return iterator(next);
		}

		return iterator();
	}

	bool Node::removeFromParent() {
		return _parent ? _parent->removeChild(this) == Result::SUCCESS : false;
	}

	size_t Node::removeAllChildren() {
		if (_childHead) {
			auto child = _childHead;
			do {
				auto next = child->_next;

				child->_prev = nullptr;
				child->_next = nullptr;
				child->_parent = nullptr;
				child->_parentChanged(child);
				Ref::unref(*child);

				child = next;
			} while (child);

			_childHead = nullptr;
			auto n = _numChildren;
			_numChildren = 0;
			return n;
		}

		return 0;
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
		using namespace srk::enum_operators;

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
		using namespace srk::enum_operators;

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
		if (!_lr.isIdentity() || _ls != decltype(_ls)::ONE || _lm.data[0][3] != 0.f || _lm.data[1][3] != 0.f || _lm.data[2][3] != 0.f) {
			_lm.set34();
			_lr.set();
			_ls.set(1.f);

			_checkNoticeUpdate(DirtyFlag::LM_WRMIM, DirtyFlag::WRMIM);
		}
	}

	void Node::updateLocalMatrix() const {
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::LM) != DirtyFlag::EMPTY) {
			_dirty &= DirtyFlag::NOT_LM;

			_lr.toMatrix(_lm);
			_lm.prependScale(_ls);
		}
	}

	void Node::updateWorldRotation() const {
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::WR) != DirtyFlag::EMPTY) {
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
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::WM) != DirtyFlag::EMPTY) {
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
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::WIM) != DirtyFlag::EMPTY) {
			_dirty &= DirtyFlag::NOT_WIM;

			updateWorldMatrix();
			_wm.invert(_iwm);
		}
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

	void Node::_worldPositionChanged(DirtyFlag oldDirty) {
		using namespace srk::enum_operators;

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

	void Node::_worldRotationChanged(DirtyFlag oldDirty) {
		using namespace srk::enum_operators;

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

	void Node::_noticeUpdate(DirtyFlag dirty) {
		auto node = _childHead;
		while (node) {
			node->_checkNoticeUpdate(dirty);
			node = node->_next;
		}
	}
}