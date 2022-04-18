#include "BlendState.h"
#include "Graphics.h"
#include "aurora/hash/xxHash.h"

namespace aurora::modules::graphics::vulkan {
	BlendState::BlendState(Graphics& graphics, bool isInternal) : IBlendState(graphics),
		_isInternal(isInternal),
		_dirty(true),
		_count(1),
		MAX_MRT_COUNT(graphics.getDeviceFeatures().simultaneousRenderTargetCount),
		_rtStatus(MAX_MRT_COUNT),
		_internalRtStatus(MAX_MRT_COUNT),
		_featureValue(0) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		memset(&_internalState, 0, sizeof(_internalState));
		_internalState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		_internalState.pAttachments = _internalRtStatus.data();
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
		if (count = Math::clamp(count, 1, MAX_MRT_COUNT); count != _count) {
			_count = count;
			_dirty = true;
		}
	}

	const RenderTargetBlendState* BlendState::getRenderTargetState(uint8_t index) const {
		return index < MAX_MRT_COUNT ? &_rtStatus[index] : nullptr;
	}

	bool BlendState::setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) {
		if (index < MAX_MRT_COUNT) {
			if (_rtStatus[index] != state) {
				_rtStatus[index] = state;
				_updateInternalState(index);
				_dirty = true;
			}

			return true;
		}
		return false;
	}

	void BlendState::_updateInternalState(uint8_t index) {
		auto& src = _rtStatus[index];
		auto& dst = _internalRtStatus[index];

		dst.blendEnable = src.enabled;
		dst.srcColorBlendFactor = _convertBlendFactor(src.func.srcColor);
		dst.dstColorBlendFactor = _convertBlendFactor(src.func.dstColor);
		dst.colorBlendOp = _convertBlendOp(src.equation.color);
		dst.srcAlphaBlendFactor = _convertBlendFactor(src.func.srcAlpha);
		dst.dstAlphaBlendFactor = _convertBlendFactor(src.func.dstAlpha);
		dst.alphaBlendOp = _convertBlendOp(src.equation.alpha);
		dst.colorWriteMask = 0;
		if (src.writeMask[0]) dst.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		if (src.writeMask[1]) dst.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		if (src.writeMask[2]) dst.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		if (src.writeMask[3]) dst.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
	}

	VkBlendFactor BlendState::_convertBlendFactor(BlendFactor factor) {
		switch (factor) {
		case BlendFactor::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::ONE:
			return VK_BLEND_FACTOR_ONE;
		case BlendFactor::SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFactor::ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFactor::SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFactor::ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendFactor::DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFactor::ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFactor::SRC_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case BlendFactor::CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case BlendFactor::ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			return VK_BLEND_FACTOR_ZERO;
		}
	}

	VkBlendOp BlendState::_convertBlendOp(BlendOp op) {
		switch (op) {
		case BlendOp::ADD:
			return VK_BLEND_OP_ADD;
		case BlendOp::SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::MIN:
			return VK_BLEND_OP_MIN;
		case BlendOp::MAX:
			return VK_BLEND_OP_MAX;
		default:
			return VK_BLEND_OP_ADD;
		}
	}

	void BlendState::update() {
		if (_dirty) {
			_internalState.attachmentCount = _count;
			_featureValue = hash::xxHash<sizeof(_featureValue) * 8>::calc<std::endian::native>(_rtStatus.data(), sizeof(RenderTargetBlendState) * _count, 0);
			_dirty = false;
		}
	}
}