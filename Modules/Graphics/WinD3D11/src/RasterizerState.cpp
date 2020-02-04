#include "RasterizerState.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	RasterizerState::RasterizerState(Graphics& graphics, bool isInternal) : IRasterizerState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_fillMode(FillMode::SOLID),
		_oldFillMode(_fillMode),
		_cullMode(CullMode::BACK),
		_oldCullMode(_cullMode),
		_frontFace(FrontFace::CW),
		_oldFrontFace(_frontFace),
		_internalState(nullptr),
		_featureValue(0) {
		if (_isInternal) _graphics->weakUnref();
		memset(&_desc, 0, sizeof(_desc));
		_desc.FillMode = _convertFillMode(_fillMode);
		_desc.CullMode = _convertCullMode(_cullMode);
		_desc.FrontCounterClockwise = _frontFace == FrontFace::CCW;
	}

	RasterizerState::~RasterizerState() {
		if (_isInternal) _graphics.weakReset();
		_releaseRes();
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
			_desc.FillMode = _convertFillMode(fill);

			_setDirty(_fillMode != _oldFillMode, DirtyFlag::FILL_MODE);
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_cullMode != cull) {
			_cullMode = cull;
			_desc.CullMode = _convertCullMode(cull);

			_setDirty(_cullMode != _oldCullMode, DirtyFlag::CULL_MODE);
		}
	}

	FrontFace RasterizerState::getFrontFace() const {
		return _frontFace;
	}

	void RasterizerState::setFrontFace(FrontFace front) {
		if (_frontFace != front) {
			_frontFace = front;
			_desc.FrontCounterClockwise = front == FrontFace::CCW;

			_setDirty(_frontFace != _oldFrontFace, DirtyFlag::FRONT_FACE);
		}
	}

	D3D11_FILL_MODE RasterizerState::_convertFillMode(FillMode mode) {
		switch (mode) {
		case FillMode::WIREFRAME:
			return D3D11_FILL_WIREFRAME;
		case FillMode::SOLID:
			return D3D11_FILL_SOLID;
		default:
			return D3D11_FILL_SOLID;
		}
	}

	D3D11_CULL_MODE RasterizerState::_convertCullMode(CullMode mode) {
		switch (mode) {
		case CullMode::NONE:
			return D3D11_CULL_NONE;
		case CullMode::FRONT:
			return D3D11_CULL_FRONT;
		case CullMode::BACK:
			return D3D11_CULL_BACK;
		default:
			return D3D11_CULL_NONE;
		}
	}

	void RasterizerState::update() {
		if (_dirty) {
			_releaseRes();
			if (SUCCEEDED(_graphics.get<Graphics>()->getDevice()->CreateRasterizerState2(&_desc, &_internalState))) {
				_oldFillMode = _fillMode;
				_oldCullMode = _cullMode;
				_oldFrontFace = _frontFace;
				_featureValue = ((uint64_t)_fillMode << 2) | ((uint64_t)_cullMode << 1) | (uint64_t)_frontFace;
				_dirty = 0;
			}
		}
	}

	void RasterizerState::_releaseRes() {
		if (_internalState) {
			_internalState->Release();
			_internalState = nullptr;
		}
		_dirty |= DirtyFlag::EMPTY;
		_featureValue = 0;
	}
}