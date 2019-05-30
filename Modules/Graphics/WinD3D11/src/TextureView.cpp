#include "TextureView.h"
#include "BaseTextureResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	TextureView::TextureView(Graphics& graphics, bool internalView) : ITextureView(graphics),
		_internalView(internalView),
		_mipBegin(0),
		_mipLevels(0),
		_createdMipLevels(0),
		_arrayBegin(0),
		_arraySize(0),
		_createdArraySize(0),
		_view(nullptr) {
	}

	TextureView::~TextureView() {
		release();
		_setRes(nullptr);
	}

	ITextureResource* TextureView::getResource() const {
		return _res.get();
	}

	const void* TextureView::getNativeView() const {
		return _view;
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

		if (res && res->getGraphics() == _graphics.get()) {
			if (auto native = (BaseTextureResource*)res->getNativeResource(); native && native->handle && (native->bindType & D3D11_BIND_SHADER_RESOURCE) && mipBegin < native->mipLevels && arrayBegin < std::max<ui32>(native->arraySize, 1)) {
				auto lastMipLevels = native->mipLevels - mipBegin;
				auto createMipLevels = mipLevels > lastMipLevels ? lastMipLevels : mipLevels;

				auto lastArraySize = native->arraySize - arrayBegin;
				if (!lastArraySize) lastArraySize = 1;
				auto createArraySize = arraySize ? (arraySize > lastArraySize ? lastArraySize : arraySize) : 0;

				D3D11_SHADER_RESOURCE_VIEW_DESC desc;
				memset(&desc, 0, sizeof(desc));
				desc.Format = native->internalFormat;

				switch (res->getType()) {
				case TextureType::TEX1D:
				{
					if (arraySize && native->arraySize) {
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
						desc.Texture1DArray.MostDetailedMip = mipBegin;
						desc.Texture1DArray.MipLevels = createMipLevels;
						desc.Texture1DArray.FirstArraySlice = arrayBegin;
						desc.Texture1DArray.ArraySize = createArraySize;
					} else {
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
						desc.Texture1D.MostDetailedMip = mipBegin;
						desc.Texture1D.MipLevels = createMipLevels;
					}

					break;
				}
				case TextureType::TEX2D:
				{
					if (arraySize && native->arraySize) {
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
						desc.Texture2DArray.MostDetailedMip = mipBegin;
						desc.Texture2DArray.MipLevels = createMipLevels;
						desc.Texture2DArray.FirstArraySlice = arrayBegin;
						desc.Texture2DArray.ArraySize = createArraySize;
					} else {
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						desc.Texture2D.MostDetailedMip = mipBegin;
						desc.Texture2D.MipLevels = createMipLevels;
					}

					break;
				}
				case TextureType::TEX3D:
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
					desc.Texture3D.MostDetailedMip = mipBegin;
					desc.Texture3D.MipLevels = createMipLevels;

					break;
				}
				default:
					break;
				}

				if (desc.ViewDimension == D3D_SRV_DIMENSION_UNKNOWN) return _createDone(false, res);

				if (FAILED(_graphics.get<Graphics>()->getDevice()->CreateShaderResourceView(native->handle, &desc, &_view))) {
					release();
					return _createDone(false, res);
				}

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
		if (!_internalView && _res != res) {
			if (_res) {
				auto native = (BaseTextureResource*)_res.get()->getNativeResource();
				native->removeView(*this);
			}
			_res = res;
			if (res) {
				auto native = (BaseTextureResource*)res->getNativeResource();
				native->addView(*this);
			}
		}
	}

	void TextureView::release() {
		if (_view) {
			_view->Release();
			_view = nullptr;
		}
		_mipBegin = 0;
		_mipLevels = 0;
		_createdMipLevels = 0;
		_arrayBegin = 0;
		_arraySize = 0;
		_createdArraySize = 0;
	}
}