#include "SimulativeTextureView.h"
#include "BaseTexture.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	SimulativeTextureView::SimulativeTextureView(Graphics& graphics) : ITextureView(graphics),
		_mipBegin(0),
		_mipLevels(0),
		_createdMipLevels(0),
		_arrayBegin(0),
		_arraySize(0),
		_createdArraySize(0),
		_handle(0) {
	}

	SimulativeTextureView::~SimulativeTextureView() {
		destroy();
	}

	bool SimulativeTextureView::isCreated() const {
		return _handle;
	}

	ITextureResource* SimulativeTextureView::getResource() const {
		return _res.get();
	}

	const void* SimulativeTextureView::getNative() const {
		return &_handle;
	}

	uint32_t SimulativeTextureView::getArraySize() const {
		return _createdArraySize;
	}

	uint32_t SimulativeTextureView::getMipLevels() const {
		return _createdMipLevels;
	}

	bool SimulativeTextureView::create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) {
		RefPtr guard(res);

		destroy();

		_mipBegin = mipBegin;
		_mipLevels = mipLevels;
		_arrayBegin = arrayBegin;
		_arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (mipBegin) {
				_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : mipBegin must be 0");
				return _createDone(false, res);
			}
			if (arrayBegin) {
				_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : arrayBegin must be 0");
				return _createDone(false, res);
			}
			if (auto native = (const BaseTexture*)res->getNative(); native && native->handle && native->mipLevels && native->arraySize) {
				if (mipLevels < native->mipLevels) {
					_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : mipLevels must >= mipLevels of res");
					return _createDone(false, res);
				}
				if (arraySize < native->arraySize) {
					_graphics.get<Graphics>()->error("openGL SimulativeTextureView::create error : arraySize must >= arraySize of res");
					return _createDone(false, res);
				}

				_handle = native->handle;

				auto createMipLevels = mipLevels > native->mipLevels ? native->mipLevels : mipLevels;

				auto lastArraySize = native->arraySize;
				if (!lastArraySize) lastArraySize = 1;
				auto createArraySize = arraySize ? (arraySize > lastArraySize ? lastArraySize : arraySize) : 0;

				_createdMipLevels = createMipLevels;
				_createdArraySize = createArraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool SimulativeTextureView::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void SimulativeTextureView::destroy() {
		if (_handle) _handle = 0;

		_mipBegin = 0;
		_mipLevels = 0;
		_createdMipLevels = 0;
		_arrayBegin = 0;
		_arraySize = 0;
		_createdArraySize = 0;
		_res.reset();
	}
}