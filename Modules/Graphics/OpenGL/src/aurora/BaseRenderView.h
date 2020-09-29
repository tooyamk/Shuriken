#pragma once

#include "Base.h"

namespace aurora::modules::graphics::gl {
	class AE_MODULE_DLL BaseRenderView {
	public:
		BaseRenderView(bool simulative);
		~BaseRenderView();

		bool simulative;
		uint32_t mipSlice;
		uint32_t arrayBegin;
		uint32_t arraySize;
		uint32_t createdArraySize;
		GLenum internalFormat;

		RefPtr<ITextureResource> res;
		GLuint handle;

		void AE_CALL destroy();
	};
}