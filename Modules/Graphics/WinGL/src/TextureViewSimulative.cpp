#include "TextureViewSimulative.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	TextureViewSimulative::TextureViewSimulative(Graphics& graphics) : ITextureView(graphics),
		_base(true) {
	}

	TextureViewSimulative::~TextureViewSimulative() {
		destroy();
	}

	bool TextureViewSimulative::isCreated() const {
		return _base.handle;
	}

	ITextureResource* TextureViewSimulative::getResource() const {
		return _base.res.get();
	}

	const void* TextureViewSimulative::getNative() const {
		return &_base;
	}

	uint32_t TextureViewSimulative::getArraySize() const {
		return _base.createdArraySize;
	}

	uint32_t TextureViewSimulative::getMipLevels() const {
		return _base.createdMipLevels;
	}

	bool TextureViewSimulative::create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) {
		RefPtr guard(res);

		destroy();

		_base.mipBegin = mipBegin;
		_base.mipLevels = mipLevels;
		_base.arrayBegin = arrayBegin;
		_base.arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (mipBegin) {
				_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : mipBegin must be 0");
				return _createDone(false, res);
			}
			if (arrayBegin) {
				_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : arrayBegin must be 0");
				return _createDone(false, res);
			}
			if (auto native = (const BaseTexture*)res->getNative(); native && native->handle) {
				if (mipLevels < native->mipLevels) {
					_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : mipLevels must >= res.mipLevels");
					return _createDone(false, res);
				}
				if (arraySize < native->arraySize) {
					_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : arraySize must >= res.arraySize");
					return _createDone(false, res);
				}

				_base.handle = native->handle;
				_base.internalFormat = native->glTexInfo.target;

				_base.createdMipLevels = native->mipLevels;
				_base.createdArraySize = native->arraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool TextureViewSimulative::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_base.res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void TextureViewSimulative::destroy() {
		_base.destroy();
	}
}