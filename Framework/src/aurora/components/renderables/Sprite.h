#pragma once

#include "aurora/components/renderables/IRenderable.h"

namespace aurora {
	class Mesh;
}

namespace aurora::components::renderables {
	class AE_FW_DLL SpriteFrame : public Ref {
	public:
        Box2f32 rect;
        Vec2f32 offset;
        Vec2f32 sourceSize;
		Vec2f32 textureSize;
		RefPtr<modules::graphics::ITextureView> texture;

        /**
         * -1 = ccw, 0 = none, 1 = cw.
         */
        uint8_t rotated = 0;
	};


	class AE_FW_DLL Sprite : public AE_COMPONENT_INHERIT(IRenderable)
	public:
		Sprite(SpriteFrame* frame = nullptr);

		virtual void AE_CALL collectRenderData(render::IRenderDataCollector& collector) const override;

		inline const SpriteFrame* AE_CALL getFrame() const {
			return _frame;
		}
		void AE_CALL setFrame(SpriteFrame* frame) {
		}

		inline Vec4f32& AE_CALL getColor() {
			return _color;
		}
		inline const Vec4f32& AE_CALL getColor() const {
			return _color;
		}
		inline void AE_CALL setColor(const Vec4f32& color) {
			_color = color;
		}

		inline Vec2f32& AE_CALL getAnchor() {
			return _anchor;
		}
		inline const Vec2f32& AE_CALL getAnchor() const {
			return _anchor;
		}
		inline void AE_CALL setAnchor(const Vec2f32& anchor) {
			_anchor = anchor;
		}

	protected:
		RefPtr<SpriteFrame> _frame;
		Vec2f32 _anchor;
		Vec4f32 _color;

		static const Mesh* AE_CALL _meshGetter(void* target);
	};
}