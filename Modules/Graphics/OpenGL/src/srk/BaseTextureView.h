#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL BaseTextureView {
	public:
		BaseTextureView(bool simulative);
		~BaseTextureView();

		bool simulative;
		size_t mipBegin;
		size_t mipLevels;
		size_t createdMipLevels;
		size_t arrayBegin;
		size_t arraySize;
		size_t createdArraySize;
		GLenum internalFormat;

		IntrusivePtr<ITextureResource> res;
		GLuint handle;

		void SRK_CALL destroy();
	};
}