#include "RenderView.h"
#include "BaseTextureResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::d3d11 {
	RenderView::RenderView(Graphics& graphics) : IRenderView(graphics),
		_mipSlice(0),
		_arrayBegin(0),
		_arraySize(0),
		_createdArraySize(0),
		_view(nullptr) {
	}

	RenderView::~RenderView() {
		destroy();
	}

	bool RenderView::isCreated() const {
		return _view;
	}

	IntrusivePtr<ITextureResource> RenderView::getResource() const {
		return _res;
	}

	const void* RenderView::getNative() const {
		return this;
	}

	uint32_t RenderView::getArraySize() const {
		return _createdArraySize;
	}

	uint32_t RenderView::getMipSlice() const {
		return _mipSlice;
	}

	bool RenderView::create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) {
		using namespace aurora::enum_operators;

		IntrusivePtr guard(res);

		destroy();
		
		_mipSlice = mipSlice;
		_arrayBegin = arrayBegin;
		_arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			auto native = (BaseTextureResource*)res->getNative();
			
			if (!native && !native->handle) {
				_graphics.get<Graphics>()->error("D3D RenderView::create error : res invalid");
				return _createDone(false, res);
			}

			if ((native->resUsage & Usage::RENDERABLE) != Usage::RENDERABLE) {
				_graphics.get<Graphics>()->error("D3D RenderView::create error : res usage must has Usage::RENDERABLE");
				return _createDone(false, res);
			}

			if (mipSlice >= native->mipLevels) {
				_graphics.get<Graphics>()->error("D3D RenderView::create error : mipSlice must < res.mipLevels");
				return _createDone(false, res);
			}

			if (native->arraySize) {
				if (arrayBegin >= native->arraySize) {
					_graphics.get<Graphics>()->error("D3D RenderView::create error : arrayBegin must < res.arraySize when res is array texture");
					return _createDone(false, res);
				}
			} else {
				if (arrayBegin) {
					_graphics.get<Graphics>()->error("D3D RenderView::create error : arrayBegin must be 0 when res is not array texture");
					return _createDone(false, res);
				}
			}

			auto lastArraySize = native->arraySize - arrayBegin;
			if (!lastArraySize) lastArraySize = 1;
			auto createArraySize = arraySize ? (arraySize > lastArraySize ? lastArraySize : arraySize) : 0;

			D3D11_RENDER_TARGET_VIEW_DESC1 desc;
			memset(&desc, 0, sizeof(desc));
			desc.Format = native->internalFormat;

			switch (res->getType()) {
			case TextureType::TEX1D:
			{
				if (arraySize && native->arraySize) {
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
					desc.Texture1DArray.MipSlice = mipSlice;
					desc.Texture1DArray.FirstArraySlice = arrayBegin;
					desc.Texture1DArray.ArraySize = createArraySize;
				} else {
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
					desc.Texture1D.MipSlice = mipSlice;
				}

				break;
			}
			case TextureType::TEX2D:
			{
				if (arraySize && native->arraySize) {
					if (native->sampleCount > 1) {
						desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
						desc.Texture2DMSArray.FirstArraySlice = arrayBegin;
						desc.Texture2DMSArray.ArraySize = createArraySize;
					} else {
						desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
						desc.Texture2DArray.MipSlice = mipSlice;
						desc.Texture2DArray.FirstArraySlice = arrayBegin;
						desc.Texture2DArray.ArraySize = createArraySize;
					}
				} else {
					if (native->sampleCount > 1) {
						desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
					} else {
						desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
						desc.Texture2D.MipSlice = mipSlice;
					}
				}

				break;
			}
			case TextureType::TEX3D:
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MipSlice = mipSlice;
				desc.Texture3D.FirstWSlice = arrayBegin;
				desc.Texture3D.WSize = arraySize;

				break;
			}
			default:
				break;
			}

			if (desc.ViewDimension == D3D_SRV_DIMENSION_UNKNOWN) {
				_graphics.get<Graphics>()->error("D3D RenderView::create error : internal format is wrong");
				return _createDone(false, res);
			}

			if (FAILED(_graphics.get<Graphics>()->getDevice()->CreateRenderTargetView1(native->handle, &desc, &_view))) {
				_graphics.get<Graphics>()->error("D3D RenderView::create error : internal view create failure");
				return _createDone(false, res);
			}

			_createdArraySize = createArraySize;

			return _createDone(true, res);
		} else {
			_graphics.get<Graphics>()->error("D3D RenderView::create error : res invalid");
			return _createDone(false, res);
		}
	}

	bool RenderView::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void RenderView::destroy() {
		if (_view) {
			_view->Release();
			_view = nullptr;
		}
		_mipSlice = 0;
		_arrayBegin = 0;
		_arraySize = 0;
		_createdArraySize = 0;
		_res.reset();
	}
}