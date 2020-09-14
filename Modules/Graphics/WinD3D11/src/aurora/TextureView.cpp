#include "TextureView.h"
#include "BaseTextureResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	TextureView::TextureView(Graphics& graphics) : ITextureView(graphics),
		_mipBegin(0),
		_mipLevels(0),
		_createdMipLevels(0),
		_arrayBegin(0),
		_arraySize(0),
		_createdArraySize(0),
		_view(nullptr) {
	}

	TextureView::~TextureView() {
		destroy();
	}

	bool TextureView::isCreated() const {
		return _view;
	}

	RefPtr<ITextureResource> TextureView::getResource() const {
		return _res;
	}

	const void* TextureView::getNative() const {
		return this;
	}

	uint32_t TextureView::getArraySize() const {
		return _createdArraySize;
	}

	uint32_t TextureView::getMipLevels() const {
		return _createdMipLevels;
	}

	bool TextureView::create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) {
		RefPtr guard(res);

		destroy();
		
		_mipBegin = mipBegin;
		_mipLevels = mipLevels;
		_arrayBegin = arrayBegin;
		_arraySize = arraySize;

		if (res && res->getGraphics() == _graphics) {
			if (auto native = (BaseTextureResource*)res->getNative(); native && native->handle && (native->bindType & D3D11_BIND_SHADER_RESOURCE) && mipBegin < native->mipLevels && arrayBegin < std::max<uint32_t>(native->arraySize, 1)) {
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
						if (native->sampleCount > 1) {
							desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
							desc.Texture2DMSArray.FirstArraySlice = arrayBegin;
							desc.Texture2DMSArray.ArraySize = createArraySize;
						} else {
							desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
							desc.Texture2DArray.MostDetailedMip = mipBegin;
							desc.Texture2DArray.MipLevels = createMipLevels;
							desc.Texture2DArray.FirstArraySlice = arrayBegin;
							desc.Texture2DArray.ArraySize = createArraySize;
						}
					} else {
						if (native->sampleCount > 1) {
							desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
						} else {
							desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
							desc.Texture2D.MostDetailedMip = mipBegin;
							desc.Texture2D.MipLevels = createMipLevels;
						}
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

				if (FAILED(_graphics.get<Graphics>()->getDevice()->CreateShaderResourceView(native->handle, &desc, &_view))) return _createDone(false, res);

				_createdMipLevels = createMipLevels;
				_createdArraySize = createArraySize;

				return _createDone(true, res);
			}
		}

		return _createDone(false, res);
	}

	bool TextureView::_createDone(bool succeeded, ITextureResource* res) {
		if (succeeded) {
			_res = res;
		} else {
			destroy();
		}
		return succeeded;
	}

	void TextureView::destroy() {
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
		_res.reset();
	}
}