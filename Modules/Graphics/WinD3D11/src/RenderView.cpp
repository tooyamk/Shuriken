#include "RenderView.h"
#include "BaseTextureResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
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

	ITextureResource* RenderView::getResource() const {
		return _res.get();
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
		RefPtr guard(res);

		destroy();
		
		_mipSlice = mipSlice;
		_arrayBegin = arrayBegin;
		_arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (auto native = (BaseTextureResource*)res->getNative(); native && native->handle && (native->bindType & D3D11_BIND_RENDER_TARGET) && mipSlice < native->mipLevels && arrayBegin < std::max<uint32_t>(native->arraySize, 1)) {
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

				if (desc.ViewDimension == D3D_SRV_DIMENSION_UNKNOWN) return _createDone(false, res);

				if (FAILED(_graphics.get<Graphics>()->getDevice()->CreateRenderTargetView1(native->handle, &desc, &_view))) return _createDone(false, res);

				_createdArraySize = createArraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
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