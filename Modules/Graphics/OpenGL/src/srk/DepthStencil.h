#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL DepthStencil : public IDepthStencil {
	public:
		DepthStencil(Graphics& graphics);
		virtual ~DepthStencil();

		virtual const void* SRK_CALL getNative() const override;
		virtual SampleCount SRK_CALL getSampleCount() const override;
		virtual const Vec2uz& SRK_CALL getSize() const override;
		virtual bool SRK_CALL create(const Vec2uz& size, DepthStencilFormat format, SampleCount sampleCount) override;
		virtual void SRK_CALL destroy() override;

		inline GLuint SRK_CALL getInternalBuffer() const {
			return _handle;
		}

		inline GLenum SRK_CALL getInternalAttachmentType() const {
			return _attachmentType;
		}

	private:
		SampleCount _sampleCount;
		Vec2uz _size;

		GLuint _handle;
		GLenum _attachmentType;

		static std::tuple<GLenum, GLenum> SRK_CALL convertInternalFormat(DepthStencilFormat fmt);
	};
}