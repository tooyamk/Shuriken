#include "RasterizerState.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	RasterizerState::RasterizerState(Graphics& graphics, bool isInternal) : IRasterizerState(graphics),
		_isInternal(isInternal),
		_dirty(true),
		_fillMode(FillMode::SOLID),
		_cullMode(CullMode::BACK),
		_frontFace(FrontFace::CW),
		_featureValue(0) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		_internalState.cullEnabled = _cullMode != CullMode::NONE;
		_internalState.fillMode = _convertFillMode(_fillMode);
		_internalState.cullMode = _convertCullMode(_cullMode);
		_internalState.frontFace = _convertFrontFace(_frontFace);
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
			_internalState.fillMode = _convertFillMode(_fillMode);
			_dirty = true;
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_cullMode != cull) {
			_cullMode = cull;
			_internalState.cullEnabled = _cullMode != CullMode::NONE;
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

	GLenum RasterizerState::_convertFillMode(FillMode mode) {
		switch (mode) {
		case FillMode::WIREFRAME:
			return GL_LINE;
		case FillMode::SOLID:
			return GL_FILL;
		default:
			return GL_FILL;
		}
	}

	GLenum RasterizerState::_convertCullMode(CullMode mode) {
		switch (mode) {
		case CullMode::FRONT:
			return GL_FRONT;
		case CullMode::BACK:
			return GL_BACK;
		default:
			return GL_BACK;
		}
	}

	GLenum RasterizerState::_convertFrontFace(FrontFace front) {
		switch (front) {
		case FrontFace::CW:
			return GL_CW;
		case FrontFace::CCW:
			return GL_CCW;
		default:
			return GL_CW;
		}
	}

	void RasterizerState::update() {
		if (_dirty) {
			_featureValue = 1U << 31 | ((uint32_t)_fillMode << 3) | ((uint32_t)_cullMode << 1) | (uint32_t)_frontFace;
			_dirty = false;
		}
	}
}