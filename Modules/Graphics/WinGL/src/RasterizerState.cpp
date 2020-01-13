#include "RasterizerState.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	RasterizerState::RasterizerState(Graphics& graphics) : IRasterizerState(graphics),
		_dirty(DirtyFlag::EMPTY),
		_featureValue(0) {
		_desc.fillMode = FillMode::SOLID;
		_desc.cullMode = CullMode::BACK;
		_desc.frontFace = FrontFace::CW;
		_oldDesc = _desc;
	}

	RasterizerState::~RasterizerState() {
	}

	FillMode RasterizerState::getFillMode() const {
		return _desc.fillMode;
	}

	void RasterizerState::setFillMode(FillMode fill) {
		if (_desc.fillMode != fill) {
			_desc.fillMode = fill;

			_setDirty(_oldDesc.fillMode != _desc.fillMode, DirtyFlag::FILL_MODE);
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _desc.cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_desc.cullMode != cull) {
			_desc.cullMode = cull;

			_setDirty(_oldDesc.cullMode != _desc.cullMode, DirtyFlag::CULL_MODE);
		}
	}

	FrontFace RasterizerState::getFrontFace() const {
		return _desc.frontFace;
	}

	void RasterizerState::setFrontFace(FrontFace front) {
		if (_desc.frontFace != front) {
			_desc.frontFace = front;

			_setDirty(_oldDesc.frontFace != _desc.frontFace, DirtyFlag::FRONT_FACE);
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
			_internalState.cullEnabled = _desc.cullMode != CullMode::NONE;
			_internalState.fillMode = _convertFillMode(_desc.fillMode);
			_internalState.cullMode = _convertCullMode(_desc.cullMode);
			_internalState.frontFace = _convertFrontFace(_desc.frontFace);
			_featureValue = calcHash(_internalState);
			_dirty = 0;
		}
	}
}