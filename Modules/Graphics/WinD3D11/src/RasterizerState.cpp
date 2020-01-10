#include "RasterizerState.h"
#include "Graphics.h"
#include "utils/hash/xxHash.h"

namespace aurora::modules::graphics::win_d3d11 {
	RasterizerState::RasterizerState(Graphics& graphics) : IRasterizerState(graphics),
		_dirty(DirtyFlag::EMPTY),
		_fillMode(FillMode::SOLID),
		_cullMode(CullMode::BACK),
		_frontFace(FrontFace::CW),
		_internalState(nullptr),
		_featureValue(0) {
		memset(&_desc, 0, sizeof(_desc));
		_desc.FillMode = _convertFillMode(_fillMode);
		_desc.CullMode = _convertCullMode(_cullMode);
		_desc.FrontCounterClockwise = _frontFace == FrontFace::CCW;
		_oldDesc = _desc;
	}

	RasterizerState::~RasterizerState() {
		_releaseRes();
	}

	FillMode RasterizerState::getFillMode() const {
		return _fillMode;
	}

	void RasterizerState::setFillMode(FillMode fill) {
		if (_fillMode != fill) {
			_fillMode = fill;
			_desc.FillMode = _convertFillMode(fill);

			_setDirty(_oldDesc.FillMode != _desc.FillMode, DirtyFlag::FILL_MODE);
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_cullMode != cull) {
			_cullMode = cull;
			_desc.CullMode = _convertCullMode(cull);

			_setDirty(_oldDesc.CullMode != _desc.CullMode, DirtyFlag::CULL_MODE);
		}
	}

	FrontFace RasterizerState::getFrontFace() const {
		return _frontFace;
	}

	void RasterizerState::setFrontFace(FrontFace front) {
		if (_frontFace != front) {
			_frontFace = front;
			_desc.FrontCounterClockwise = front == FrontFace::CCW;

			_setDirty(_oldDesc.FrontCounterClockwise != _desc.FrontCounterClockwise, DirtyFlag::FRONT_FACE);
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
				_oldDesc = _desc;
				_featureValue = hash::xxHash::calc<sizeof(_featureValue) * 8, std::endian::native>((uint8_t*)&_desc, sizeof(_desc), 0);
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