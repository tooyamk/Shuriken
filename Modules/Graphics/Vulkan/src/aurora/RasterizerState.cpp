#include "RasterizerState.h"
#include "Graphics.h"

namespace aurora::modules::graphics::vulkan {
	RasterizerState::RasterizerState(Graphics& graphics, bool isInternal) : IRasterizerState(graphics),
		_isInternal(isInternal),
		_dirty(true),
		_fillMode(FillMode::SOLID),
		_cullMode(CullMode::BACK),
		_frontFace(FrontFace::CW),
		_featureValue(0) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		memset(&_internalState, 0, sizeof(_internalState));
		_internalState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		_internalState.polygonMode = _convertFillMode(_fillMode);
		_internalState.cullMode = _convertCullMode(_cullMode);
		_internalState.frontFace = _convertFrontFace(_frontFace);
		_internalState.depthClampEnable = VK_TRUE;
		_internalState.rasterizerDiscardEnable = VK_TRUE;
		_internalState.depthBiasEnable = VK_TRUE;
		_internalState.lineWidth = 1.0f;
	}

	RasterizerState::~RasterizerState() {
		if (_isInternal) _graphics.reset<false>();
	}

	const void* RasterizerState::getNative() const {
		return this;
	}

	FillMode RasterizerState::getFillMode() const {
		return _fillMode;
	}

	void RasterizerState::setFillMode(FillMode fill) {
		if (_fillMode != fill) {
			_fillMode = fill;
			_internalState.polygonMode = _convertFillMode(_fillMode);
			_dirty = true;
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_cullMode != cull) {
			_cullMode = cull;
			_internalState.cullMode = _convertCullMode(_cullMode);
			_dirty = true;
		}
	}

	FrontFace RasterizerState::getFrontFace() const {
		return _frontFace;
	}

	void RasterizerState::setFrontFace(FrontFace front) {
		if (_frontFace != front) {
			_frontFace = front;
			_internalState.frontFace = _convertFrontFace(_frontFace);
			_dirty = true;
		}
	}

	VkPolygonMode RasterizerState::_convertFillMode(FillMode mode) {
		switch (mode) {
		case FillMode::WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case FillMode::SOLID:
			return VK_POLYGON_MODE_FILL;
		default:
			return VK_POLYGON_MODE_FILL;
		}
	}

	VkCullModeFlagBits RasterizerState::_convertCullMode(CullMode mode) {
		switch (mode) {
		case CullMode::NONE:
			return VK_CULL_MODE_NONE;
		case CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		default:
			return VK_CULL_MODE_NONE;
		}
	}

	VkFrontFace RasterizerState::_convertFrontFace(FrontFace mode) {
		return mode == FrontFace::CW ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	void RasterizerState::update() {
		if (_dirty) {
			_featureValue = 1U << 31 | ((uint32_t)_fillMode << 3) | ((uint32_t)_cullMode << 1) | (uint32_t)_frontFace;
			_dirty = false;
		}
	}
}