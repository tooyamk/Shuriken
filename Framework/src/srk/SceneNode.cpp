#include "SceneNode.h"
#include "srk/components/IComponent.h"

namespace srk {
	SceneNode::SceneNode() :
		name(),
		_parent(nullptr),
		_root(this),
		_prev(nullptr),
		_next(nullptr),
		_childHead(nullptr),
		_numChildren(0),
		_lr(),
		_ls(decltype(_ls)::ONE),
		_lm(),
		_wr(),
		_wm(),
		_iwm(),
		_dirty(DirtyFlag::EMPTY) {
	}

	SceneNode::~SceneNode() {
		removeAllChildren();
	}

	SceneNode::Result SceneNode::addChild(SceneNode* child) {
		if (!child) return Result::NOT_NULL;
		if (child->_parent) return Result::CANNOT_HAS_PARENT;
		if (child == _root) return Result::CANNOT_IS_ROOT_OF_SELF;

		_addChild(child);

		return Result::SUCCESS;
	}

	SceneNode::Result SceneNode::insertChild(SceneNode* child, SceneNode* before) {
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

	SceneNode::Result SceneNode::removeChild(SceneNode* child) {
		if (!child) return Result::NOT_NULL;
		if (child->_parent != this) return Result::ISNOT_CHILD_OF_SELF;

		_removeNode(child);
		child->_parentChanged(child);
		Ref::unref(*child);

		return Result::SUCCESS;
	}

	SceneNode::iterator SceneNode::removeChild(const iterator& itr) {
		if (auto child = itr._node; child && child->_parent == this) {
			auto next = child->_next;

			_removeNode(child);
			child->_parentChanged(child);
			Ref::unref(*child);

			return iterator(next);
		}

		return iterator();
	}

	bool SceneNode::removeFromParent() {
		return _parent ? _parent->removeChild(this) == Result::SUCCESS : false;
	}

	size_t SceneNode::removeAllChildren() {
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

	void SceneNode::setLocalPosition(const float32_t(&p)[3]) {
		_lm.set<MatrixHint::NONE, Math::Data2DDesc(0, 3, 0, 0, 3, 1)>((float32_t(&)[3][1])p);

		_checkNoticeUpdate(DirtyFlag::WM_IWM);
	}

	void SceneNode::localTranslate(const float32_t(&p)[3]) {
		updateLocalMatrix();
		_lm.prependTranslation(p);

		_checkNoticeUpdate(DirtyFlag::WM_IWM);
	}

	void SceneNode::setLocalRotation(const Quaternion<float32_t>& q) {
		_lr.set(q);

		_checkNoticeUpdate(DirtyFlag::LM_WM_IWM_WQ, DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::localRotate(const Quaternion<float32_t>& q) {
		_lr.prepend(q);

		_checkNoticeUpdate(DirtyFlag::LM_WM_IWM_WQ, DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::setLocalScale(const float32_t(&s)[3]) {
		_ls.set(s);

		_checkNoticeUpdate(DirtyFlag::LM_WM_IWM, DirtyFlag::WM_IWM);
	}

	void SceneNode::setLocalMatrix(const Matrix3x4f32& m) {
		using namespace srk::enum_operators;

		_lm.set(m);
		_localDecomposition();

		_checkNoticeUpdateNow((_dirty & DirtyFlag::NOT_LM) | DirtyFlag::WM_IWM_WQ, DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::setLocalTRS(const float32_t(&pos)[3], const Quaternion<float32_t>& rot, const float32_t(&scale)[3]) {
		_lm.set<MatrixHint::NONE, Math::Data2DDesc(0, 3, 0, 0, 3, 1)>((float32_t(&)[3][1])pos);
		_lr.set(rot);
		_ls.set(scale);

		_checkNoticeUpdate(DirtyFlag::LM_WM_IWM_WQ, DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::parentTranslate(const float32_t(&p)[3]) {
		_lm.appendTranslation(p);

		_checkNoticeUpdate(DirtyFlag::WM_IWM);
	}

	void SceneNode::parentRotate(const Quaternion<float32_t>& q) {
		_lr.append(q);

		_checkNoticeUpdate(DirtyFlag::LM_WM_IWM_WQ, DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::setWorldPosition(const float32_t(&p)[3]) {
		auto old = _dirty;
		updateWorldMatrix();
		_wm.set<MatrixHint::NONE, Math::Data2DDesc(0, 3, 0, 0, 3, 1)>((float32_t(&)[3][1])p);

		_worldPositionChanged(old);
	}

	void SceneNode::worldTranslate(const float32_t(&p)[3]) {
		auto old = _dirty;
		updateWorldMatrix();
		_wm.prependTranslation(p);

		_worldPositionChanged(old);
	}

	void SceneNode::setWorldRotation(const Quaternion<float32_t>& q) {
		_wr.set(q);

		_worldRotationChanged(_dirty);
	}

	void SceneNode::worldRotate(const Quaternion<float32_t>& q) {
		auto old = _dirty;
		updateWorldRotation();
		_wr.prepend(q);

		_worldRotationChanged(old);
	}

	void SceneNode::setWorldMatrix(const Matrix3x4f32& m) {
		using namespace srk::enum_operators;

		_wm.set(m);

		auto now = _dirty & DirtyFlag::NOT_WM;

		if (_parent) {
			now &= DirtyFlag::NOT_IWM;
			_wm.invert(_iwm);
			_wm.append(_parent->getInverseWorldMatrix(), _lm);
		} else {
			now |= DirtyFlag::IWM;
			_lm.set(_wm);
		}

		_localDecomposition();

		_checkNoticeUpdateNow((now & DirtyFlag::NOT_LM) | DirtyFlag::WQ, DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::setIdentity() {
		if (!_lr.isIdentity() || _ls != decltype(_ls)::ONE || _lm(0, 3) != 0.f || _lm(1, 3) != 0.f || _lm(2, 3) != 0.f) {
			_lm.identity();
			_lr.set();
			_ls = 1.f;

			_checkNoticeUpdate(DirtyFlag::LM_WM_IWM_WQ, DirtyFlag::WM_IWM_WQ);
		}
	}

	void SceneNode::updateLocalMatrix() const {
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::LM) != DirtyFlag::EMPTY) {
			_dirty &= DirtyFlag::NOT_LM;

			_lm.fromQuaternion<MatrixHint::NONE>(_lr.data);
			_lm.prependScale(_ls.data);
		}
	}

	void SceneNode::updateWorldRotation() const {
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::WQ) != DirtyFlag::EMPTY) {
			_dirty &= DirtyFlag::NOT_WQ;

			auto p = _parent;
			if (_parent) {
				_lr.append(_parent->getWorldRotation(), _wr);
			} else {
				_wr.set(_lr);
			}
		}
	}

	void SceneNode::updateWorldMatrix() const {
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::WM) != DirtyFlag::EMPTY) {
			_dirty &= DirtyFlag::NOT_WM;

			updateLocalMatrix();
			
			if (_parent) {
				_lm.append(_parent->getWorldMatrix(), _wm);
			} else {
				_wm.set(_lm);
			}
		}
	}

	void SceneNode::updateInverseWorldMatrix() const {
		using namespace srk::enum_operators;

		if ((_dirty & DirtyFlag::IWM) != DirtyFlag::EMPTY) {
			_dirty &= DirtyFlag::NOT_IWM;

			updateWorldMatrix();
			_wm.invert(_iwm);
		}
	}

	void SceneNode::getLocalRotationFromWorld(const SceneNode& node, const Quaternion<float32_t>& worldRot, Quaternion<float32_t>& dst) {
		if (node._parent) {
			Quaternion<float32_t> i(nullptr);
			node._parent->getWorldRotation().invert(i);
			worldRot.append(i, dst);
		} else {
			dst.set(worldRot);
		}
	}

	void SceneNode::_addNode(SceneNode* child) {
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

	void SceneNode::_insertNode(SceneNode* child, SceneNode* before) {
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

	void SceneNode::_removeNode(SceneNode* child) {
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

	void SceneNode::_worldPositionChanged(DirtyFlag oldDirty) {
		using namespace srk::enum_operators;

		float32_t p[3] = { _wm.data[0][3], _wm.data[1][3], _wm.data[2][3] };
		if (_parent) _parent->getInverseWorldMatrix().transformPoint<nullptr, nullptr, Math::Hint::IDENTITY_IF_NOT_EXIST | Math::Hint::MEM_OVERLAP>(p, p);
		_lm.set<MatrixHint::NONE, Math::Data2DDesc(0, 3, 0, 0, 3, 1)>((float32_t(&)[3][1])p);

		_dirty |= DirtyFlag::IWM;
		if (oldDirty != _dirty) _noticeUpdate(DirtyFlag::WM_IWM);
	}

	void SceneNode::_worldRotationChanged(DirtyFlag oldDirty) {
		using namespace srk::enum_operators;

		if (_parent) {
			Quaternion<float32_t> i(nullptr);
			_parent->getWorldRotation().invert(i);
			_wr.append(i, _lr);
		} else {
			_lr.set(_wr);
		}

		_dirty &= DirtyFlag::NOT_WQ;
		_dirty |= DirtyFlag::LM_WM_IWM;
		if (oldDirty != _dirty) _noticeUpdate(DirtyFlag::WM_IWM_WQ);
	}

	void SceneNode::_noticeUpdate(DirtyFlag dirty) {
		auto node = _childHead;
		while (node) {
			node->_checkNoticeUpdate(dirty);
			node = node->_next;
		}
	}
}