#include "BaseTextureView.h"

namespace aurora::modules::graphics::win_gl {
	BaseTextureView::BaseTextureView(bool simulative) :
		simulative(simulative),
		mipBegin(0),
		mipLevels(0),
		createdMipLevels(0),
		arrayBegin(0),
		arraySize(0),
		createdArraySize(0),
		internalFormat(GL_NONE),
		handle(0) {
	}

	BaseTextureView::~BaseTextureView() {
		destroy();
	}

	void BaseTextureView::destroy() {
		if (handle) {
			if (!simulative) glDeleteTextures(1, &handle);
			handle = 0;
		}

		mipBegin = 0;
		mipLevels = 0;
		createdMipLevels = 0;
		arrayBegin = 0;
		arraySize = 0;
		createdArraySize = 0;
		internalFormat = GL_NONE;
		res.reset();
	}
}