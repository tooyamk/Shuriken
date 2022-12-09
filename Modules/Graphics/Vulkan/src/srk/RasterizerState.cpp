#include "RasterizerState.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	RasterizerState::RasterizerState(Graphics& graphics, bool isInternal) : IRasterizerState(graphics),
		_isInternal(isInternal),
		_dirty(true) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		memset(&_internalState, 0, sizeof(_internalState));
		_internalState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		_internalState.polygonMode = Graphics::convertFillMode(_desc.fillMode);
		_internalState.cullMode = Graphics::convertCullMode(_desc.cullMode);
		_internalState.frontFace = Graphics::convertFrontFace(_desc.frontFace);
		_internalState.depthClampEnable = VK_FALSE;
		_internalState.rasterizerDiscardEnable = VK_FALSE;
		_internalState.depthBiasEnable = VK_FALSE;
		_internalState.depthBiasConstantFactor = 0.0f; // Optional
		_internalState.depthBiasClamp = 0.0f; // Optional
		_internalState.depthBiasSlopeFactor = 0.0f; // Optional
		_internalState.lineWidth = 1.0f;
	}

	RasterizerState::~RasterizerState() {
		if (_isInternal) _graphics.reset<false>();
	}

	const void* RasterizerState::getNative() const {
		return this;
	}

	FillMode RasterizerState::getFillMode() const {
		return _desc.fillMode;
	}

	void RasterizerState::setFillMode(FillMode fill) {
		if (_desc.fillMode != fill) {
			_desc.fillMode = fill;
			_internalState.polygonMode = Graphics::convertFillMode(_desc.fillMode);
			_dirty = true;
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _desc.cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_desc.cullMode != cull) {
			_desc.cullMode = cull;
			_internalState.cullMode = Graphics::convertCullMode(_desc.cullMode);
			_dirty = true;
		}
	}

	FrontFace RasterizerState::getFrontFace() const {
		return _desc.frontFace;
	}

	void RasterizerState::setFrontFace(FrontFace front) {
		if (_desc.frontFace != front) {
			_desc.frontFace = front;
			_internalState.frontFace = Graphics::convertFrontFace(_desc.frontFace);
			_dirty = true;
		}
	}

	bool RasterizerState::getScissorEnabled() const {
		return _desc.scissorEnabled;
	}

	void RasterizerState::setScissorEnabled(bool enabled) {
		if (_desc.scissorEnabled != enabled) {
			_desc.scissorEnabled = enabled;
			_dirty = true;
		}
	}

	void RasterizerState::update() {
		if (_dirty) {
			auto desc = _desc;
			desc.scissorEnabled = false;
			_featureValue.set(desc);
			_dirty = false;
		}
	}
}