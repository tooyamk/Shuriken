#include "TextureView.h"
#include "BaseTextureResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	TextureView::TextureView(Graphics& graphics) : ITextureView(graphics),
		_mipBegin(0),
		_mipLevels(0),
		_createdMipLevels(0),
		_view(nullptr) {
	}

	TextureView::~TextureView() {
		_release();
		_setRes(nullptr);
	}

	ITextureResource* TextureView::getResource() const {
		return _res.get();
	}

	const void* TextureView::getNative() const {
		return _view;
	}

	ui32 TextureView::getMipLevels() const {
		return _createdMipLevels;
	}

	bool TextureView::create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels) {
		_release();

		_mipBegin = mipBegin;
		_mipLevels = mipLevels;

		if (res && res->getGraphics() == _graphics.get()) {
			auto native = (BaseTextureResource*)res->getNative();
			if (native && (native->bindType & D3D11_BIND_SHADER_RESOURCE) && mipBegin < res->getMipLevels()) {
				auto lastMipLevels = res->getMipLevels() - mipBegin;
				auto createMipLevels = mipLevels > lastMipLevels ? mipLevels = lastMipLevels : mipLevels;

				D3D11_SHADER_RESOURCE_VIEW_DESC desc;
				memset(&desc, 0, sizeof(desc));
				desc.Format = native->internalFormat;

				switch (res->getType()) {
				case TextureType::TEX1D:
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
					desc.Texture1D.MostDetailedMip = mipBegin;
					desc.Texture1D.MipLevels = createMipLevels;

					break;
				}
				case TextureType::TEX2D:
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MostDetailedMip = mipBegin;
					desc.Texture2D.MipLevels = createMipLevels;
					//desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					//desc.Texture2DArray.MostDetailedMip = mipBegin;
					//desc.Texture2DArray.MipLevels = createMipLevels;
					//desc.Texture2DArray.FirstArraySlice = 0;
					//desc.Texture2DArray.ArraySize = 1;

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
					_release();
					return _createDone(false, res);
				}

				_createdMipLevels = createMipLevels;

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
				auto native = (BaseTextureResource*)_res.get()->getNative();
				native->removeView(*this);
			}
			_res = res;
			if (res) {
				auto native = (BaseTextureResource*)res->getNative();
				native->addView(*this, std::bind(&TextureView::_onResRecreated, this));
			}
		}
	}

	void TextureView::_release() {
		if (_view) {
			_view->Release();
			_view = nullptr;
		}
		_mipBegin = 0;
		_mipLevels = 0;
		_createdMipLevels = 0;
	}

	void TextureView::_onResRecreated() {
		create(_res.get(), _mipBegin, _mipLevels);
	}
}