#include "RasterizerState.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	RasterizerState::RasterizerState(Graphics& graphics, bool isInternal) : IRasterizerState(graphics),
		_isInternal(isInternal),
		_dirty(true) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		_internalState.cullEnabled = _desc.cullMode != CullMode::NONE;
		_internalState.scissorEnabled = _desc.scissorEnabled;
		_internalState.fillMode = Graphics::convertFillMode(_desc.fillMode);
		_internalState.cullMode = Graphics::convertCullMode(_desc.cullMode);
		_internalState.frontFace = Graphics::convertFrontFace(_desc.frontFace);
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
			_internalState.fillMode = Graphics::convertFillMode(_desc.fillMode);
			_dirty = true;
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _desc.cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_desc.cullMode != cull) {
			_desc.cullMode = cull;
			_internalState.cullEnabled = _desc.cullMode != CullMode::NONE;
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
			_internalState.scissorEnabled = enabled;
			_dirty = true;
		}
	}

	void RasterizerState::update() {
		if (_dirty) {
			_featureValue.set(_desc);
			_dirty = false;
		}
	}
}