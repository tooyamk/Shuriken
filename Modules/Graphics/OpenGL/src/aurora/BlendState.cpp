#include "BlendState.h"
#include "Graphics.h"

namespace aurora::modules::graphics::gl {
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

	uint16_t BlendState::_convertBlendFactor(BlendFactor factor) {
		switch (factor) {
		case BlendFactor::ZERO:
			return GL_ZERO;
		case BlendFactor::ONE:
			return GL_ONE;
		case BlendFactor::SRC_COLOR:
			return GL_SRC_COLOR;
		case BlendFactor::ONE_MINUS_SRC_COLOR:
			return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::SRC_ALPHA:
			return GL_SRC_ALPHA;
		case BlendFactor::ONE_MINUS_SRC_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DST_COLOR:
			return GL_DST_COLOR;
		case BlendFactor::ONE_MINUS_DST_COLOR:
			return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::DST_ALPHA:
			return GL_DST_ALPHA;
		case BlendFactor::ONE_MINUS_DST_ALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case BlendFactor::SRC_ALPHA_SATURATE:
			return GL_SRC_ALPHA_SATURATE;
		case BlendFactor::CONSTANT_COLOR:
			return GL_CONSTANT_COLOR;
		case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::SRC1_COLOR:
			return GL_SRC1_COLOR;
		case BlendFactor::ONE_MINUS_SRC1_COLOR:
			return GL_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::SRC1_ALPHA:
			return GL_SRC1_ALPHA;
		case BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return GL_ONE_MINUS_SRC1_ALPHA;
		default:
			return GL_ZERO;
		}
	}

	uint16_t BlendState::_convertBlendOp(BlendOp op) {
		switch (op) {
		case BlendOp::ADD:
			return GL_FUNC_ADD;
		case BlendOp::SUBTRACT:
			return GL_FUNC_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::MIN:
			return GL_MIN;
		case BlendOp::MAX:
			return GL_MAX;
		default:
			return GL_FUNC_ADD;
		}
	}

	void BlendState::_updateInternalState(uint8_t index) {
		auto& rt = _status[index];
		
		auto& func = rt.state.func;
		auto& internalFunc = rt.internalFunc;
		internalFunc.srcColor = _convertBlendFactor(func.srcColor);
		internalFunc.dstColor = _convertBlendFactor(func.dstColor);
		internalFunc.srcAlpha = _convertBlendFactor(func.srcAlpha);
		internalFunc.dstAlpha = _convertBlendFactor(func.dstAlpha);

		auto& op = rt.state.equation;
		auto& internalOp = rt.internalOp;
		internalOp.color = _convertBlendOp(op.color);
		internalOp.alpha = _convertBlendOp(op.alpha);

		memcpy(rt.internalWriteMask.rgba, rt.state.writeMask.data, sizeof(rt.internalWriteMask.rgba));
	}
}