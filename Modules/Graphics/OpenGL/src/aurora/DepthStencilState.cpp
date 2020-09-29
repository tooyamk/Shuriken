#include "DepthStencilState.h"
#include "Graphics.h"

namespace aurora::modules::graphics::gl {
	DepthStencilState::DepthStencilState(Graphics& graphics, bool isInternal) : IDepthStencilState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_stencilFeatureValue(0) {
		if (_isInternal) Ref::unref<false>(*_graphics);
	}

	DepthStencilState::~DepthStencilState() {
		if (_isInternal) _graphics.reset<false>();
	}

	const void* DepthStencilState::getNative() const {
		return this;
	}

	const DepthState& DepthStencilState::getDepthState() const {
		return _depthState;
	}

	void DepthStencilState::setDepthState(const DepthState& depthState) {
		if (_depthState != depthState) {
			_depthState = depthState;
			_updateDepth();

			_setDirty(_depthState != _oldDepthState, DirtyFlag::DEPTH);
		}
	}

	void DepthStencilState::_updateDepth() {
		_internalDepthState.enabled = _depthState.enabled;
		_internalDepthState.writeable = _depthState.writeable;
		_internalDepthState.func = Graphics::convertComparisonFunc(_depthState.func);
	}

	const StencilState& DepthStencilState::getStencilState() const {
		return _stencilState;
	}

	void DepthStencilState::setStencilState(const StencilState& stencilState) {
		if (_stencilState != stencilState) {
			_stencilState = stencilState;
			_updateStencil();

			_setDirty(_stencilState != _oldStencilState, DirtyFlag::STENCIL);
		}
	}

	void DepthStencilState::_updateStencil() {
		_internalStencilState.enabled = _stencilState.enabled;
		_updateStecnilFace(_internalStencilState.face.front, _stencilState.face.front);
		_updateStecnilFace(_internalStencilState.face.back, _stencilState.face.back);
	}

	void DepthStencilState::_updateStecnilFace(InternalStencilFaceState& desc, const StencilFaceState& state) {
		desc.op.depthFail = _convertStencilOp(state.op.depthFail);
		desc.op.fail = _convertStencilOp(state.op.fail);
		desc.op.pass = _convertStencilOp(state.op.pass);
		desc.mask.read = state.mask.read;
		desc.mask.write = state.mask.write;
		desc.func = Graphics::convertComparisonFunc(state.func);
	}

	GLenum DepthStencilState::_convertStencilOp(StencilOp op) {
		switch (op) {
		case StencilOp::KEEP:
			return GL_KEEP;
		case StencilOp::ZERO:
			return GL_ZERO;
		case StencilOp::REPLACE:
			return GL_REPLACE;
		case StencilOp::INCR_CLAMP:
			return GL_INCR;
		case StencilOp::DECR_CLAMP:
			return GL_DECR;
		case StencilOp::INCR_WRAP:
			return GL_INCR_WRAP;
		case StencilOp::DECR_WRAP:
			return GL_DECR_WRAP;
		case StencilOp::INVERT:
			return GL_INVERT;
		default:
			return GL_KEEP;
		}
	}

	void DepthStencilState::update() {
		if (_dirty) {
			constexpr DirtyType de = DirtyFlag::DEPTH | DirtyFlag::EMPTY;
			constexpr DirtyType se = DirtyFlag::STENCIL | DirtyFlag::EMPTY;

			if (_dirty & de) _updateDepth();
			if (_dirty & se) {
				_updateStencil();
				_stencilFeatureValue = calcHash(_internalStencilState.face);
			}
			_dirty = 0;
		}
	}
}