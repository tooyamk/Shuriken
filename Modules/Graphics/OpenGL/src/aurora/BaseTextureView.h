#pragma once

#include "Base.h"

namespace aurora::modules::graphics::gl {
	class AE_MODULE_DLL BaseTextureView {
	public:
		BaseTextureView(bool simulative);
		~BaseTextureView();

		bool simulative;
		uint32_t mipBegin;
		uint32_t mipLevels;
		uint32_t createdMipLevels;
		uint32_t arrayBegin;
		uint32_t arraySize;
		uint32_t createdArraySize;
		GLenum internalFormat;

		IntrusivePtr<ITextureResource> res;
		GLuint handle;

		void AE_CALL destroy();
	};
}