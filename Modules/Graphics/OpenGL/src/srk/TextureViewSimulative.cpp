#include "TextureViewSimulative.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	TextureViewSimulative::TextureViewSimulative(Graphics& graphics) : ITextureView(graphics),
		_base(true) {
	}

	TextureViewSimulative::~TextureViewSimulative() {
		destroy();
	}

	bool TextureViewSimulative::isCreated() const {
		return _base.handle;
	}

	IntrusivePtr<ITextureResource> TextureViewSimulative::getResource() const {
		return _base.res;
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
		IntrusivePtr guard(res);

		destroy();

		_base.mipBegin = mipBegin;
		_base.mipLevels = mipLevels;
		_base.arrayBegin = arrayBegin;
		_base.arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (mipBegin) {
				_graphics.get<Graphics>()->error("OpenGL TextureView(Simulative)::create error : mipBegin must be 0");
				return _createDone(false, res);
			}

			if (arrayBegin) {
				_graphics.get<Graphics>()->error("OpenGL TextureView(Simulative)::create error : arrayBegin must be 0");
				return _createDone(false, res);
			}

			auto native = (const BaseTexture*)res->getNative();

			if (!native && !native->handle) {
				_graphics.get<Graphics>()->error("OpenGL TextureView(Simulative)::create error : res invalid");
				return _createDone(false, res);
			}

			if (mipLevels < native->mipLevels) {
				_graphics.get<Graphics>()->error("OpenGL TextureView(Simulative)::create error : mipLevels must >= res.mipLevels");
				return _createDone(false, res);
			}

			if (arraySize < native->arraySize) {
				_graphics.get<Graphics>()->error("OpenGL TextureView(Simulative)::create error : arraySize must >= res.arraySize");
				return _createDone(false, res);
			}

			_base.handle = native->handle;
			_base.internalFormat = native->glTexInfo.target;

			_base.createdMipLevels = native->mipLevels;
			_base.createdArraySize = native->arraySize;

			return _createDone(true, res);
		} else {
			_graphics.get<Graphics>()->error("OpenGL TextureView(Simulative)::create error : res invalid");
			return _createDone(false, res);
		}
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