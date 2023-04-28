#include "DepthStencilState.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	DepthStencilState::DepthStencilState(Graphics& graphics, bool isInternal) : IDepthStencilState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_internalState(nullptr) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		_updateDepth();
		_updateStencil();
	}

	DepthStencilState::~DepthStencilState() {
		if (_isInternal) _graphics.reset<false>();
		_releaseRes();
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
		_desc.DepthEnable = _depthState.enabled;
		_desc.DepthWriteMask = _depthState.writeable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		_desc.DepthFunc = Graphics::convertComparisonFunc(_depthState.func);
	}

	const StencilState& DepthStencilState::getStencilState() const {
		return _stencilState;
	}

	void DepthStencilState::setStencilState(const StencilState& stencilState) {
		if (_stencilState.face.front.ref != stencilState.face.front.ref) {
			_stencilState.face.front.ref = stencilState.face.front.ref;
			_setDirty(_stencilState.face.front.ref != _oldStencilState.face.front.ref, DirtyFlag::STENCIL_REF);
		}
		if (_isStecnilBaseNotEqual(_stencilState, stencilState)) {
			_stencilState = stencilState;
			auto& back = _stencilState.face.back;
			back.mask.read = std::numeric_limits<decltype(back.mask.read)>::max();
			back.mask.write = std::numeric_limits<decltype(back.mask.write)>::max();
			back.ref = 0;

			_updateStencil();
			_setDirty(_isStecnilBaseNotEqual(_stencilState, _oldStencilState), DirtyFlag::STENCIL);
		}
	}

	void DepthStencilState::_updateStencil() {
		_desc.StencilEnable = _stencilState.enabled;
		_desc.StencilReadMask = _stencilState.face.front.mask.read;
		_desc.StencilWriteMask = _stencilState.face.front.mask.write;
		_updateStecnilFace(_desc.FrontFace, _stencilState.face.front);
		_updateStecnilFace(_desc.BackFace, _stencilState.face.back);
	}

	void DepthStencilState::_updateStecnilFace(D3D11_DEPTH_STENCILOP_DESC& desc, const StencilFaceState& state) {
		desc.StencilDepthFailOp = Graphics::convertStencilOp(state.op.depthFail);
		desc.StencilFailOp = Graphics::convertStencilOp(state.op.fail);
		desc.StencilPassOp = Graphics::convertStencilOp(state.op.pass);
		desc.StencilFunc = Graphics::convertComparisonFunc(state.func);
	}

	void DepthStencilState::update() {
		if (_dirty) {
			constexpr DirtyType recreateFlags = DirtyFlag::EMPTY | DirtyFlag::DEPTH | DirtyFlag::STENCIL;
			if (_dirty & recreateFlags) {
				_releaseRes();
				_graphics.get<Graphics>()->getDevice()->CreateDepthStencilState(&_desc, &_internalState);
			}

			_oldDepthState = _depthState;
			_oldStencilState = _stencilState;

			_featureValue.set(_depthState, _stencilState);

			_dirty = 0;
		}
	}

	bool DepthStencilState::_isStecnilBaseNotEqual(const StencilState& lhs, const StencilState& rhs) {
		return lhs.enabled != rhs.enabled ||
			lhs.face.front.funcAndOpFeatureValue != rhs.face.front.funcAndOpFeatureValue ||
			lhs.face.front.mask.featureValue != rhs.face.front.mask.featureValue ||
			lhs.face.back.funcAndOpFeatureValue != rhs.face.back.funcAndOpFeatureValue;
	}

	void DepthStencilState::_releaseRes() {
		if (_internalState) {
			_internalState->Release();
			_internalState = nullptr;
		}
		_dirty |= DirtyFlag::EMPTY;
		_featureValue = nullptr;
	}
}