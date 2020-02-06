#include "RenderViewSimulative.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	RenderViewSimulative::RenderViewSimulative(Graphics& graphics) : IRenderView(graphics),
		_base(true) {
	}

	RenderViewSimulative::~RenderViewSimulative() {
		destroy();
	}

	bool RenderViewSimulative::isCreated() const {
		return _base.handle;
	}

	ITextureResource* RenderViewSimulative::getResource() const {
		return _base.res.get();
	}

	const void* RenderViewSimulative::getNative() const {
		return &_base;
	}

	uint32_t RenderViewSimulative::getArraySize() const {
		return _base.createdArraySize;
	}

	uint32_t RenderViewSimulative::getMipSlice() const {
		return _base.mipSlice;
	}

	bool RenderViewSimulative::create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) {
		RefPtr guard(res);

		destroy();

		_base.mipSlice = mipSlice;
		_base.arrayBegin = arrayBegin;
		_base.arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (mipSlice) {
				_graphics.get<Graphics>()->error("openGL SimulativeRenderView::create error : mipBegin must be 0");
				return _createDone(false, res);
			}
			if (arrayBegin) {
				_graphics.get<Graphics>()->error("openGL SimulativeRenderView::create error : arrayBegin must be 0");
				return _createDone(false, res);
			}
			if (auto native = (const BaseTexture*)res->getNative(); native && native->handle && (native->resUsage & Usage::RENDERABLE) == Usage::RENDERABLE) {
				if (arraySize < native->arraySize) {
					_graphics.get<Graphics>()->error("openGL SimulativeRenderView::create error : arraySize must >= res.arraySize");
					return _createDone(false, res);
				}

				_base.handle = native->handle;
				_base.internalFormat = native->glTexInfo.target;

				_base.createdArraySize = native->arraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool RenderViewSimulative::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_base.res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void RenderViewSimulative::destroy() {
		_base.destroy();
	}
}