#include "BlendState.h"
#include "Graphics.h"
#include "aurora/hash/xxHash.h"

namespace aurora::modules::graphics::win_d3d11 {
	BlendState::BlendState(Graphics& graphics, bool isInternal) : IBlendState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_desc({ 0 }),
		_internalState(nullptr),
		_featureValue(0) {
		if (_isInternal) _graphics->unref<false>();
		_oldIndependentBlendEnabled = _desc.IndependentBlendEnable;
		for (uint8_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) _setRenderTargetState(i, _rtStatus[i]);
	}

	BlendState::~BlendState() {
		if (_isInternal) _graphics.reset<false>();
		_releaseRes();
	}

	const void* BlendState::getNative() const {
		return this;
	}

	bool BlendState::isIndependentBlendEnabled() const {
		return _desc.IndependentBlendEnable;
	}

	void BlendState::setIndependentBlendEnabled(bool enalbed) {
		if (bool(_desc.IndependentBlendEnable) != enalbed) {
			_desc.IndependentBlendEnable = enalbed;

			_setDirty(_oldIndependentBlendEnabled != bool(_desc.IndependentBlendEnable), DirtyFlag::INDEPENDENT_BLEND_ENABLE);
		}
	}

	const RenderTargetBlendState* BlendState::getRenderTargetState(uint8_t index) const {
		return index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ? &_rtStatus[index] : nullptr;
	}

	bool BlendState::setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) {
		if (index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) {
			if (_rtStatus[index] != state) {
				_rtStatus[index] = state;

				_setRenderTargetState(index, state);

				_setDirty(_oldRtStatus[index] != _rtStatus[index], DirtyFlag::RT_STATE);
			}

			return true;
		}
		return false;
	}

	void BlendState::_setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) {
		auto& stateDesc = _desc.RenderTarget[index];
		stateDesc.BlendEnable = state.enabled;
		stateDesc.SrcBlend = _convertBlendFactor(state.func.srcColor);
		stateDesc.SrcBlendAlpha = _convertBlendFactor(state.func.srcAlpha);
		stateDesc.DestBlend = _convertBlendFactor(state.func.dstColor);
		stateDesc.DestBlendAlpha = _convertBlendFactor(state.func.dstAlpha);
		stateDesc.BlendOp = _convertBlendOp(state.equation.color);
		stateDesc.BlendOpAlpha = _convertBlendOp(state.equation.alpha);
		stateDesc.RenderTargetWriteMask =
			(state.writeMask.data[0] ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
			(state.writeMask.data[1] ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
			(state.writeMask.data[2] ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
			(state.writeMask.data[3] ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
	}

	D3D11_BLEND BlendState::_convertBlendFactor(BlendFactor factor) {
		switch (factor) {
		case BlendFactor::ZERO:
			return D3D11_BLEND_ZERO;
		case BlendFactor::ONE:
			return D3D11_BLEND_ONE;
		case BlendFactor::SRC_COLOR:
			return D3D11_BLEND_SRC_COLOR;
		case BlendFactor::ONE_MINUS_SRC_COLOR:
			return D3D11_BLEND_INV_SRC_COLOR;
		case BlendFactor::SRC_ALPHA:
			return D3D11_BLEND_SRC_ALPHA;
		case BlendFactor::ONE_MINUS_SRC_ALPHA:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case BlendFactor::DST_COLOR:
			return D3D11_BLEND_DEST_COLOR;
		case BlendFactor::ONE_MINUS_DST_COLOR:
			return D3D11_BLEND_INV_DEST_COLOR;
		case BlendFactor::DST_ALPHA:
			return D3D11_BLEND_DEST_ALPHA;
		case BlendFactor::ONE_MINUS_DST_ALPHA:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case BlendFactor::SRC_ALPHA_SATURATE:
			return D3D11_BLEND_SRC_ALPHA_SAT;
		case BlendFactor::CONSTANT_COLOR:
			return D3D11_BLEND_BLEND_FACTOR;
		case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return D3D11_BLEND_INV_BLEND_FACTOR;
		case BlendFactor::SRC1_COLOR:
			return D3D11_BLEND_SRC1_COLOR;
		case BlendFactor::ONE_MINUS_SRC1_COLOR:
			return D3D11_BLEND_INV_SRC1_COLOR;
		case BlendFactor::SRC1_ALPHA:
			return D3D11_BLEND_SRC1_ALPHA;
		case BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return D3D11_BLEND_INV_SRC1_ALPHA;
		default:
			return D3D11_BLEND_ZERO;
		}
	}

	D3D11_BLEND_OP BlendState::_convertBlendOp(BlendOp op) {
		switch (op) {
		case BlendOp::ADD:
			return D3D11_BLEND_OP_ADD;
		case BlendOp::SUBTRACT:
			return D3D11_BLEND_OP_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return D3D11_BLEND_OP_REV_SUBTRACT;
		case BlendOp::MIN:
			return D3D11_BLEND_OP_MIN;
		case BlendOp::MAX:
			return D3D11_BLEND_OP_MAX;
		default:
			return D3D11_BLEND_OP_ADD;
		}
	}

	void BlendState::update() {
		if (_dirty) {
			_releaseRes();
			if (SUCCEEDED(_graphics.get<Graphics>()->getDevice()->CreateBlendState1(&_desc, &_internalState))) {
				_oldIndependentBlendEnabled = _desc.IndependentBlendEnable;
				memcpy(&_oldRtStatus, &_rtStatus, sizeof(_rtStatus));
				_featureValue = hash::xxHash::calc<sizeof(_featureValue) * 8, std::endian::native>(&_desc, _desc.IndependentBlendEnable ? sizeof(_desc) : sizeof(_desc) - sizeof(_desc.RenderTarget[0]) * 7, 0);
				_dirty = 0;
			}
		}
	}

	void BlendState::_releaseRes() {
		if (_internalState) {
			_internalState->Release();
			_internalState = nullptr;
		}
		_dirty |= DirtyFlag::EMPTY;
		_featureValue = 0;
	}
}