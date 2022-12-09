#include "BlendState.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	BlendState::BlendState(Graphics& graphics, bool isInternal) : IBlendState(graphics),
		_isInternal(isInternal),
		_count(1),
		MAX_MRT_COUNT(graphics.getDeviceFeatures().simultaneousRenderTargetCount),
		_status(MAX_MRT_COUNT) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		for (uint8_t i = 0; i < MAX_MRT_COUNT; ++i) _updateInternalState(i);
	}

	BlendState::~BlendState() {
		if (_isInternal) _graphics.reset<false>();
	}

	const void* BlendState::getNative() const {
		return this;
	}

	const Vec4f32& BlendState::getConstants() const {
		return _constants;
	}

	void BlendState::setConstants(const Vec4f32& val) {
		_constants = val;
	}

	uint8_t BlendState::getCount() const {
		return _count;
	}

	void BlendState::setCount(uint8_t count) {
		_count = Math::clamp(count, 1, MAX_MRT_COUNT);
	}

	const RenderTargetBlendState* BlendState::getRenderTargetState(uint8_t index) const {
		return index < MAX_MRT_COUNT ? &_status[index].state : nullptr;
	}

	bool BlendState::setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) {
		if (index < MAX_MRT_COUNT) {
			if (_status[index].state != state) {
				_status[index].state = state;
				_updateInternalState(index);
			}

			return true;
		}
		return false;
	}

	void BlendState::_updateInternalState(uint8_t index) {
		auto& rt = _status[index];
		
		auto& func = rt.state.func;
		auto& internalFunc = rt.internalFunc;
		internalFunc.srcColor = Graphics::convertBlendFactor(func.srcColor);
		internalFunc.dstColor = Graphics::convertBlendFactor(func.dstColor);
		internalFunc.srcAlpha = Graphics::convertBlendFactor(func.srcAlpha);
		internalFunc.dstAlpha = Graphics::convertBlendFactor(func.dstAlpha);

		auto& op = rt.state.equation;
		auto& internalOp = rt.internalOp;
		internalOp.color = Graphics::convertBlendOp(op.color);
		internalOp.alpha = Graphics::convertBlendOp(op.alpha);

		memcpy(rt.internalWriteMask.rgba, rt.state.writeMask.data, sizeof(rt.internalWriteMask.rgba));
	}
}