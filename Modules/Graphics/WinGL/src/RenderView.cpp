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
			auto native = (const BaseTexture*)res->getNative();

			if (!native && !native->handle) {
				_graphics.get<Graphics>()->error("openGL RenderView::create error : res invalid");
				return _createDone(false, res);
			}

			if ((native->resUsage & Usage::RENDERABLE) != Usage::RENDERABLE) {
				_graphics.get<Graphics>()->error("openGL RenderView::create error : res usage must has Usage::RENDERABLE");
				return _createDone(false, res);
			}

			if (mipSlice >= native->mipLevels) {
				_graphics.get<Graphics>()->error("openGL RenderView::create error : mipSlice must < res.mipLevels");
				return _createDone(false, res);
			}

			if (native->arraySize) {
				if (arrayBegin >= native->arraySize) {
					_graphics.get<Graphics>()->error("openGL RenderView::create error : arrayBegin must < res.arraySize when res is array texture");
					return _createDone(false, res);
				}
			} else {
				if (arrayBegin) {
					_graphics.get<Graphics>()->error("openGL RenderView::create error : arrayBegin must be 0 when res is not array texture");
					return _createDone(false, res);
				}
			}

			auto lastArraySize = native->arraySize - arrayBegin;
			if (!lastArraySize) lastArraySize = 1;
			auto createArraySize = arraySize ? (arraySize > lastArraySize ? lastArraySize : arraySize) : 0;

			_base.internalFormat = GL_NONE;
			switch (res->getType()) {
			case TextureType::TEX1D:
				_base.internalFormat = arraySize && native->arraySize ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
				break;
			case TextureType::TEX2D:
			{
				if (native->sampleCount > 1) {
					_base.internalFormat = arraySize && native->arraySize ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_MULTISAMPLE;
				} else {
					_base.internalFormat = arraySize && native->arraySize ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
				}

				break;
			}
			case TextureType::TEX3D:
				_base.internalFormat = GL_TEXTURE_3D;
				break;
			default:
				break;
			}

			if (_base.internalFormat == GL_NONE) {
				_graphics.get<Graphics>()->error("openGL RenderView::create error : unknow internal format");
				return _createDone(false, res);
			}

			glGenTextures(1, &_base.handle);
			glTextureView(_base.handle, _base.internalFormat, native->handle, native->glTexInfo.internalFormat, mipSlice, 1, arrayBegin, createArraySize ? createArraySize : 1);

			_base.createdArraySize = createArraySize;

			return _createDone(true, res);
		} else {
			_graphics.get<Graphics>()->error("openGL RenderView::create error : res invalid");
			return _createDone(false, res);
		}
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