#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL BaseRenderView {
	public:
		BaseRenderView(bool simulative);
		~BaseRenderView();

		bool simulative;
		size_t mipSlice;
		size_t arrayBegin;
		size_t arraySize;
		size_t createdArraySize;
		GLenum internalFormat;

		IntrusivePtr<ITextureResource> res;
		GLuint handle;

		void SRK_CALL destroy();
	};
}