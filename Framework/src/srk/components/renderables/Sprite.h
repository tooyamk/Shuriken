#pragma once

#include "srk/components/renderables/IRenderable.h"

namespace srk {
	class Mesh;
}

namespace srk::components::renderables {
	class SRK_FW_DLL SpriteFrame {
		SRK_REF_OBJECT(SpriteFrame)
	public:
        Box2f32 rect;
        Vec2f32 offset;
        Vec2f32 sourceSize;
		Vec2f32 textureSize;
		IntrusivePtr<modules::graphics::ITextureView> texture;

        /**
         * -1 = ccw, 0 = none, 1 = cw.
         */
        uint8_t rotated = 0;
	};


	class SRK_FW_DLL Sprite : public SRK_COMPONENT_INHERIT(IRenderable)
	public:
		Sprite(SpriteFrame* frame = nullptr);

		virtual void SRK_CALL collectRenderData(render::IRenderDataCollector& collector) const override;

		inline const SpriteFrame* SRK_CALL getFrame() const {
			return _frame;
		}
		void SRK_CALL setFrame(SpriteFrame* frame) {
		}

		inline Vec4f32& SRK_CALL getColor() {
			return _color;
		}
		inline const Vec4f32& SRK_CALL getColor() const {
			return _color;
		}
		inline void SRK_CALL setColor(const Vec4f32& color) {
			_color = color;
		}

		inline Vec2f32& SRK_CALL getAnchor() {
			return _anchor;
		}
		inline const Vec2f32& SRK_CALL getAnchor() const {
			return _anchor;
		}
		inline void SRK_CALL setAnchor(const Vec2f32& anchor) {
			_anchor = anchor;
		}

	protected:
		IntrusivePtr<SpriteFrame> _frame;
		Vec2f32 _anchor;
		Vec4f32 _color;

		static const Mesh* SRK_CALL _meshGetter(void* target);
	};
}