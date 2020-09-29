#include "DepthStencilState.h"
#include "Graphics.h"

namespace aurora::modules::graphics::d3d11 {
	DepthStencilState::DepthStencilState(Graphics& graphics, bool isInternal) : IDepthStencilState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_internalState(nullptr),
		_featureValue(0) {
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
		if (_stencilState != stencilState) {
			_stencilState = stencilState;
			_updateStencil();

			_setDirty(_stencilState != _oldStencilState, DirtyFlag::STENCIL);
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
		desc.StencilDepthFailOp = _convertStencilOp(state.op.depthFail);
		desc.StencilFailOp = _convertStencilOp(state.op.fail);
		desc.StencilPassOp = _convertStencilOp(state.op.pass);
		desc.StencilFunc = Graphics::convertComparisonFunc(state.func);
	}

	D3D11_STENCIL_OP DepthStencilState::_convertStencilOp(StencilOp op) {
		switch (op) {
		case StencilOp::KEEP:
			return D3D11_STENCIL_OP_KEEP;
		case StencilOp::ZERO:
			return D3D11_STENCIL_OP_ZERO;
		case StencilOp::REPLACE:
			return D3D11_STENCIL_OP_REPLACE;
		case StencilOp::INCR_CLAMP:
			return D3D11_STENCIL_OP_INCR_SAT;
		case StencilOp::DECR_CLAMP:
			return D3D11_STENCIL_OP_DECR_SAT;
		case StencilOp::INCR_WRAP:
			return D3D11_STENCIL_OP_INCR;
		case StencilOp::DECR_WRAP:
			return D3D11_STENCIL_OP_DECR;
		case StencilOp::INVERT:
			return D3D11_STENCIL_OP_INVERT;
		default:
			return D3D11_STENCIL_OP_KEEP;
		}
	}

	void DepthStencilState::update() {
		if (_dirty) {
			_releaseRes();
			if (SUCCEEDED(_graphics.get<Graphics>()->getDevice()->CreateDepthStencilState(&_desc, &_internalState))) {
				_oldDepthState = _depthState;
				_oldStencilState = _stencilState;

				_featureValue = 0x1ULL << 63;
				if (_depthState.enabled) {
					_featureValue |= 0x1ULL << 54;
					if (_depthState.writeable) _featureValue |= 0x1ULL << 53;
					_featureValue |= (uint64_t)_depthState.func << 49;
				}
				if (_stencilState.enabled) {
					_featureValue |= 0x1ULL << 48;
					_featureValue |= (uint64_t)(*((uint16_t*)&_stencilState.face.front.mask)) << 32;
					_featureValue |= (uint64_t)_stencilState.face.front.op.fail << 28;
					_featureValue |= (uint64_t)_stencilState.face.front.op.depthFail << 24;
					_featureValue |= (uint64_t)_stencilState.face.front.op.pass << 20;
					_featureValue |= (uint64_t)_stencilState.face.front.func << 16;
					_featureValue |= (uint64_t)_stencilState.face.back.op.fail << 12;
					_featureValue |= (uint64_t)_stencilState.face.back.op.depthFail << 8;
					_featureValue |= (uint64_t)_stencilState.face.back.op.pass << 4;
					_featureValue |= (uint64_t)_stencilState.face.back.func << 0;
				}

				_dirty = 0;
			}
		}
	}

	void DepthStencilState::_releaseRes() {
		if (_internalState) {
			_internalState->Release();
			_internalState = nullptr;
		}
		_dirty |= DirtyFlag::EMPTY;
		_featureValue = 0;
	}
}