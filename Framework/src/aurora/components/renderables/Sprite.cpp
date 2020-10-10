#include "Sprite.h"
#include "aurora/render/IRenderDataCollector.h"

namespace aurora::components::renderables {
	Sprite::Sprite(SpriteFrame* frame) :
		_frame(frame),
		_anchor(0.5f),
		_color(1.0f) {
	}

	void Sprite::collectRenderData(render::IRenderDataCollector& collector) const {
		if (_frame) {
			collector.data.mesh.set(&Sprite::_meshGetter, (void*)this);

			collector.commit();

			collector.data.mesh = nullptr;
		}
	}

	const Mesh* Sprite::_meshGetter(void* target) {
		return nullptr;
	}
}