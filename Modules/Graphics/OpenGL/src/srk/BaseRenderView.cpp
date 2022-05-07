#include "BaseRenderView.h"

namespace srk::modules::graphics::gl {
	BaseRenderView::BaseRenderView(bool simulative) :
		simulative(simulative),
		mipSlice(0),
		arrayBegin(0),
		arraySize(0),
		createdArraySize(0),
		internalFormat(GL_NONE),
		handle(0) {
	}

	BaseRenderView::~BaseRenderView() {
		destroy();
	}

	void BaseRenderView::destroy() {
		if (handle) {
			if (!simulative) glDeleteTextures(1, &handle);
			handle = 0;
		}

		mipSlice = 0;
		arrayBegin = 0;
		arraySize = 0;
		createdArraySize = 0;
		internalFormat = GL_NONE;
		res.reset();
	}
}