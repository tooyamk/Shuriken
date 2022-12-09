#include "RasterizerState.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	RasterizerState::RasterizerState(Graphics& graphics, bool isInternal) : IRasterizerState(graphics),
		_isInternal(isInternal),
		_dirty(DirtyFlag::EMPTY),
		_internalState(nullptr) {
		if (_isInternal) Ref::unref<false>(*_graphics);
		memset(&_desc, 0, sizeof(_desc));
		_desc.FillMode = Graphics::convertFillMode(_cur.fillMode);
		_desc.CullMode = Graphics::convertCullMode(_cur.cullMode);
		_desc.FrontCounterClockwise = _cur.frontFace == FrontFace::CCW;
		_desc.DepthClipEnable = true;
		_desc.ScissorEnable = _cur.scissorEnabled ? TRUE : FALSE;
	}

	RasterizerState::~RasterizerState() {
		if (_isInternal) _graphics.reset<false>();
		_releaseRes();
	}

	const void* RasterizerState::getNative() const {
		return this;
	}

	FillMode RasterizerState::getFillMode() const {
		return _cur.fillMode;
	}

	void RasterizerState::setFillMode(FillMode fill) {
		if (_cur.fillMode != fill) {
			_cur.fillMode = fill;
			_desc.FillMode = Graphics::convertFillMode(fill);

			_setDirty(_cur.fillMode != _old.fillMode, DirtyFlag::FILL_MODE);
		}
	}

	CullMode RasterizerState::getCullMode() const {
		return _cur.cullMode;
	}

	void RasterizerState::setCullMode(CullMode cull) {
		if (_cur.cullMode != cull) {
			_cur.cullMode = cull;
			_desc.CullMode = Graphics::convertCullMode(cull);

			_setDirty(_cur.cullMode != _old.cullMode, DirtyFlag::CULL_MODE);
		}
	}

	FrontFace RasterizerState::getFrontFace() const {
		return _cur.frontFace;
	}

	void RasterizerState::setFrontFace(FrontFace front) {
		if (_cur.frontFace != front) {
			_cur.frontFace = front;
			_desc.FrontCounterClockwise = front == FrontFace::CCW;

			_setDirty(_cur.frontFace != _old.frontFace, DirtyFlag::FRONT_FACE);
		}
	}

	bool RasterizerState::getScissorEnabled() const {
		return _cur.scissorEnabled;
	}

	void RasterizerState::setScissorEnabled(bool enabled) {
		if (_cur.scissorEnabled != enabled) {
			_cur.scissorEnabled = enabled;
			_desc.ScissorEnable = enabled ? TRUE : FALSE;

			_setDirty(_cur.scissorEnabled != _old.scissorEnabled, DirtyFlag::SCISSOR);
		}
	}

	void RasterizerState::update() {
		if (_dirty) {
			_releaseRes();
			if (SUCCEEDED(_graphics.get<Graphics>()->getDevice()->CreateRasterizerState2(&_desc, &_internalState))) {
				_old = _cur;
				_featureValue.set(_cur);
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