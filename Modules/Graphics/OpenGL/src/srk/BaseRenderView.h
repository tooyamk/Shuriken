#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL BaseRenderView {
	public:
		BaseRenderView(bool simulative);
		~BaseRenderView();

		bool simulative;
		uint32_t mipSlice;
		uint32_t arrayBegin;
		uint32_t arraySize;
		uint32_t createdArraySize;
		GLenum internalFormat;

		IntrusivePtr<ITextureResource> res;
		GLuint handle;

		void SRK_CALL destroy();
	};
}