#include "RenderView.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	RenderView::RenderView(Graphics& graphics) : IRenderView(graphics),
		_base(false) {
	}

	RenderView::~RenderView() {
		destroy();
	}

	bool RenderView::isCreated() const {
		return _base.handle;
	}

	ITextureResource* RenderView::getResource() const {
		return _base.res.get();
	}

	const void* RenderView::getNative() const {
		return &_base;
	}

	uint32_t RenderView::getArraySize() const {
		return _base.createdArraySize;
	}

	uint32_t RenderView::getMipSlice() const {
		return _base.mipSlice;
	}

	bool RenderView::create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) {
		RefPtr guard(res);

		destroy();

		_base.mipSlice = mipSlice;
		_base.arrayBegin = arrayBegin;
		_base.arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (auto native = (const BaseTexture*)res->getNative(); native && native->handle && mipSlice < native->mipLevels && (native->arraySize ? arrayBegin < native->arraySize : arrayBegin == 0)) {
				auto lastArraySize = native->arraySize - arrayBegin;
				if (!lastArraySize) lastArraySize = 1;
				auto createArraySize = arraySize ? (arraySize > lastArraySize ? lastArraySize : arraySize) : 0;

				_base.internalFormat = GL_NONE;
				switch (res->getType()) {
				case TextureType::TEX1D:
					_base.internalFormat = arraySize && native->arraySize ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
					break;
				case TextureType::TEX2D:
					_base.internalFormat = arraySize && native->arraySize ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
					break;
				case TextureType::TEX3D:
					_base.internalFormat = GL_TEXTURE_3D;
					break;
				default:
					break;
				}

				if (_base.internalFormat == GL_NONE) return _createDone(false, res);

				glGenTextures(1, &_base.handle);
				glTextureView(_base.handle, _base.internalFormat, native->handle, native->glTexInfo.internalFormat, mipSlice, 1, arrayBegin, createArraySize ? createArraySize : 1);

				_base.createdArraySize = createArraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool RenderView::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_base.res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void RenderView::destroy() {
		_base.destroy();
	}
}