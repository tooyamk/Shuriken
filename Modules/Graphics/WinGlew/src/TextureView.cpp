#include "TextureView.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	TextureView::TextureView(Graphics& graphics) : ITextureView(graphics),
		_mipBegin(0),
		_mipLevels(0),
		_createdMipLevels(0),
		_arrayBegin(0),
		_arraySize(0),
		_createdArraySize(0),
		_handle(0) {
	}

	TextureView::~TextureView() {
		release();
		_setRes(nullptr);
	}

	ITextureResource* TextureView::getResource() const {
		return _res.get();
	}

	const void* TextureView::getNativeView() const {
		return &_handle;
	}

	ui32 TextureView::getArraySize() const {
		return _createdArraySize;
	}

	ui32 TextureView::getMipLevels() const {
		return _createdMipLevels;
	}

	bool TextureView::create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels, ui32 arrayBegin, ui32 arraySize) {
		release();

		_mipBegin = mipBegin;
		_mipLevels = mipLevels;
		_arrayBegin = arrayBegin;
		_arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			auto native = (const BaseTexture*)res->getNativeResource();
			if (native && native->handle && mipBegin < native->mipLevels && arrayBegin <= native->arraySize) {
				auto lastMipLevels = native->mipLevels - mipBegin;
				auto createMipLevels = mipLevels > lastMipLevels ? lastMipLevels : mipLevels;

				auto lastArraySize = native->arraySize - arrayBegin;
				if (!lastArraySize) lastArraySize = 1;
				auto createArraySize = arraySize ? (arraySize > lastArraySize ? lastArraySize : arraySize) : 0;

				GLenum target = 0;
				switch (res->getType()) {
				case TextureType::TEX1D:
					target = arraySize ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
					break;
				case TextureType::TEX2D:
					target = arraySize ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
					break;
				case TextureType::TEX3D:
					target = GL_TEXTURE_3D;
					break;
				default:
					break;
				}

				if (!target) return _createDone(false, res);

				glGenTextures(1, &_handle);
				glTextureView(_handle, target, native->handle, 1, mipBegin, createMipLevels, arrayBegin, createArraySize ? createArraySize : 1);

				_createdMipLevels = createMipLevels;
				_createdArraySize = createArraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool TextureView::_createDone(bool succeeded, ITextureResource* res) {
		_setRes(res);
		return succeeded;
	}

	void TextureView::_setRes(ITextureResource* res) {
		if (_res != res) {
			if (_res) {
				auto native = (BaseTexture*)_res.get()->getNativeResource();
				native->removeView(*this);
			}
			_res = res;
			if (res) {
				auto native = (BaseTexture*)res->getNativeResource();
				native->addView(*this, std::bind(&TextureView::_onResRecreated, this));
			}
		}
	}

	void TextureView::release() {
		if (_handle) {
			glDeleteTextures(1, &_handle);
			_handle = 0;
		}
		_mipBegin = 0;
		_mipLevels = 0;
		_createdMipLevels = 0;
		_arrayBegin = 0;
		_arraySize = 0;
		_createdArraySize = 0;
	}

	void TextureView::_onResRecreated() {
		create(_res.get(), _mipBegin, _mipLevels, _arrayBegin, _arraySize);
	}
}