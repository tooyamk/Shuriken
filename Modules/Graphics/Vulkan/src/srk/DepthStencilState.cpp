#include "DepthStencilState.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	DepthStencilState::DepthStencilState(Graphics& graphics, bool isInternal) : IDepthStencilState(graphics),
		_isInternal(isInternal),
		_dirty(true) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		memset(&_internalState, 0, sizeof(_internalState));
		_internalState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		_internalState.depthBoundsTestEnable = VK_FALSE;
	}

	DepthStencilState::~DepthStencilState() {
		if (_isInternal) _graphics.reset<false>();
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
			_dirty = true;
		}
	}

	void DepthStencilState::_updateDepth() {
		_internalState.depthTestEnable = _depthState.enabled ? VK_TRUE : VK_FALSE;
		_internalState.depthWriteEnable = _depthState.writeable ? VK_TRUE : VK_FALSE;
		_internalState.depthCompareOp = Graphics::convertCompareOp(_depthState.func);
	}

	const StencilState& DepthStencilState::getStencilState() const {
		return _stencilState;
	}

	void DepthStencilState::setStencilState(const StencilState& stencilState) {
		if (_stencilState != stencilState) {
			_stencilState = stencilState;
			_updateStencil();
			_dirty = true;
		}
	}

	void DepthStencilState::_updateStencil() {
		_internalState.stencilTestEnable = _stencilState.enabled ? VK_TRUE : VK_FALSE;
		_updateStecnilFace(_internalState.front, _stencilState.face.front);
		_updateStecnilFace(_internalState.back, _stencilState.face.back);
	}

	void DepthStencilState::_updateStecnilFace(VkStencilOpState& vkState, const StencilFaceState& state) {
		vkState.depthFailOp = Graphics::convertStencilOp(state.op.depthFail);
		vkState.failOp = Graphics::convertStencilOp(state.op.fail);
		vkState.passOp = Graphics::convertStencilOp(state.op.pass);
		vkState.compareOp = Graphics::convertCompareOp(state.func);
		vkState.writeMask = state.mask.write;
		vkState.compareMask = state.mask.read;
		vkState.reference = state.ref;
	}

	void DepthStencilState::update() {
		if (_dirty) {
			_featureValue.set(_depthState, _stencilState);
			_dirty = false;
		}
	}
}