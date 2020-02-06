#include "TextureView.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	TextureView::TextureView(Graphics& graphics) : ITextureView(graphics),
		_base(false) {
	}

	TextureView::~TextureView() {
		destroy();
	}

	bool TextureView::isCreated() const {
		return _base.handle;
	}

	ITextureResource* TextureView::getResource() const {
		return _base.res.get();
	}

	const void* TextureView::getNative() const {
		return &_base;
	}

	uint32_t TextureView::getArraySize() const {
		return _base.createdArraySize;
	}

	uint32_t TextureView::getMipLevels() const {
		return _base.createdMipLevels;
	}

	bool TextureView::create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) {
		RefPtr guard(res);

		destroy();

		_base.mipBegin = mipBegin;
		_base.mipLevels = mipLevels;
		_base.arrayBegin = arrayBegin;
		_base.arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (auto native = (const BaseTexture*)res->getNative(); native && native->handle && mipBegin < native->mipLevels && (native->arraySize ? arrayBegin < native->arraySize : arrayBegin == 0)) {
				auto lastMipLevels = native->mipLevels - mipBegin;
				auto createMipLevels = mipLevels > lastMipLevels ? lastMipLevels : mipLevels;

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
				glTextureView(_base.handle, _base.internalFormat, native->handle, native->glTexInfo.internalFormat, mipBegin, createMipLevels, arrayBegin, createArraySize ? createArraySize : 1);

				_base.createdMipLevels = createMipLevels;
				_base.createdArraySize = createArraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool TextureView::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_base.res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void TextureView::destroy() {
		_base.destroy();
	}
}