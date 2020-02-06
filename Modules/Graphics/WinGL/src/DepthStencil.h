#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_gl {
	class Graphics;

	class AE_MODULE_DLL DepthStencil : public IDepthStencil {
	public:
		DepthStencil(Graphics& graphics);
		virtual ~DepthStencil();

		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL isMultisampling() const override;
		virtual const Vec2ui32& AE_CALL getSize() const override;
		virtual bool AE_CALL create(const Vec2ui32& size, DepthStencilFormat format, bool multisampling) override;
		virtual void AE_CALL destroy() override;

		inline GLuint AE_CALL getInternalBuffer() const {
			return _handle;
		}

		inline GLenum AE_CALL getInternalAttachmentType() const {
			return _attachmentType;
		}

	private:
		bool _isMultisampling;
		Vec2ui32 _size;

		GLuint _handle;
		GLenum _attachmentType;

		static std::tuple<GLenum, GLenum> AE_CALL convertInternalFormat(DepthStencilFormat fmt);
	};
}