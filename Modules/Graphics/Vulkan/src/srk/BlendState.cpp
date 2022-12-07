#include "BlendState.h"
#include "Graphics.h"
#include "srk/hash/xxHash.h"

namespace srk::modules::graphics::vulkan {
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
		dst.srcColorBlendFactor = Graphics::convertBlendFactor(src.func.srcColor);
		dst.dstColorBlendFactor = Graphics::convertBlendFactor(src.func.dstColor);
		dst.colorBlendOp = Graphics::convertBlendOp(src.equation.color);
		dst.srcAlphaBlendFactor = Graphics::convertBlendFactor(src.func.srcAlpha);
		dst.dstAlphaBlendFactor = Graphics::convertBlendFactor(src.func.dstAlpha);
		dst.alphaBlendOp = Graphics::convertBlendOp(src.equation.alpha);
		dst.colorWriteMask = 0;
		if (src.writeMask[0]) dst.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		if (src.writeMask[1]) dst.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		if (src.writeMask[2]) dst.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		if (src.writeMask[3]) dst.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
	}

	void BlendState::update() {
		if (_dirty) {
			_internalState.attachmentCount = _count;
			_featureValue = hash::xxHash<sizeof(_featureValue) * 8>::calc<std::endian::native>(_rtStatus.data(), sizeof(RenderTargetBlendState) * _count, 0);
			_dirty = false;
		}
	}
}