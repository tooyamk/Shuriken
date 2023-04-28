#include "BlendState.h"
#include "Graphics.h"
#include "srk/hash/xxHash.h"

namespace srk::modules::graphics::d3d11 {
	BlendState::BlendState(Graphics& graphics, bool isInternal) : IBlendState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_count(1),
		_oldCount(_count),
		_desc({ 0 }),
		_internalState(nullptr),
		_featureValue(0) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		for (uint8_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) _setRenderTargetState(i, _rtStatus[i]);
	}

	BlendState::~BlendState() {
		if (_isInternal) _graphics.reset<false>();
		_releaseRes();
	}

	const void* BlendState::getNative() const {
		return this;
	}

	const Vec4f32& BlendState::getConstants() const {
		return _constants;
	}

	void BlendState::setConstants(const Vec4f32& val) {
		if (_constants != val) {
			_constants = val;

			_setDirty(true, DirtyFlag::CONSTANTS);
		}
	}

	uint8_t BlendState::getCount() const {
		return _count;
	}

	void BlendState::setCount(uint8_t count) {
		count = Math::clamp(count, 1, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
		if (_count == count) return;

		_count = count;
		auto independent = count > 1;
		if (bool(_desc.IndependentBlendEnable) != independent) {
			_desc.IndependentBlendEnable = independent;

			_setDirty(_oldCount != _count, DirtyFlag::COUNT);
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
		stateDesc.SrcBlend = Graphics::convertBlendFactor(state.func.srcColor);
		stateDesc.SrcBlendAlpha = Graphics::convertBlendFactor(state.func.srcAlpha);
		stateDesc.DestBlend = Graphics::convertBlendFactor(state.func.dstColor);
		stateDesc.DestBlendAlpha = Graphics::convertBlendFactor(state.func.dstAlpha);
		stateDesc.BlendOp = Graphics::convertBlendOp(state.equation.color);
		stateDesc.BlendOpAlpha = Graphics::convertBlendOp(state.equation.alpha);
		stateDesc.RenderTargetWriteMask =
			(state.writeMask.data[0] ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
			(state.writeMask.data[1] ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
			(state.writeMask.data[2] ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
			(state.writeMask.data[3] ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
	}

	void BlendState::update() {
		if (_dirty) {
			constexpr DirtyType recreateFlags = DirtyFlag::EMPTY | DirtyFlag::COUNT | DirtyFlag::RT_STATE;
			if (_dirty & recreateFlags) {
				_releaseRes();
				_graphics.get<Graphics>()->getDevice()->CreateBlendState1(&_desc, &_internalState);
			}
			
			_oldCount = _count;
			memcpy(&_oldRtStatus, &_rtStatus, sizeof(_rtStatus));

			hash::xxHash<64> hasher;
			hasher.begin(0);
			hasher.update(&_desc, sizeof(_desc) - sizeof(_desc.RenderTarget[0]) * (D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT - _count));
			hasher.update(&_constants, sizeof(_constants));
			_featureValue = hasher.digest();

			_dirty = 0;
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