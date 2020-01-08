#include "BlendState.h"
#include "Graphics.h"
#include "utils/hash/xxHash.h"

namespace aurora::modules::graphics::win_glew {
	BlendState::BlendState(Graphics& graphics) : IBlendState(graphics) {
	}

	BlendState::~BlendState() {
	}

	const RenderTargetBlendState BlendState::DEFAULT_RT_STATE = RenderTargetBlendState();

	bool BlendState::isIndependentBlendEnabled() const {
		return _desc.independentBlendEnabled;
	}

	void BlendState::setIndependentBlendEnabled(bool enalbed) {
		_desc.independentBlendEnabled = enalbed;
	}

	const RenderTargetBlendState& BlendState::getRenderTargetState(uint8_t index) const {
		return index < NUM_RTS ? _desc.renderTarget[index] : DEFAULT_RT_STATE;
	}

	void BlendState::setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) {
		if (index < NUM_RTS) _desc.renderTarget[index] = state;
	}
}