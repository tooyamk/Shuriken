#include "DepthStencil.h"
#include "Graphics.h"

namespace aurora::modules::graphics::d3d11 {
	DepthStencil::DepthStencil(Graphics& graphics, bool internalView) : IDepthStencil(graphics),
		_sampleCount(0),
		_view(nullptr) {
		if (_isInternal) Ref::unref<false>(*_graphics);
	}

	DepthStencil::~DepthStencil() {
		if (_isInternal) _graphics.reset<false>();
		destroy();
	}

	const void* DepthStencil::getNative() const {
		return this;
	}

	SampleCount DepthStencil::getSampleCount() const {
		return _sampleCount;
	}

	const Vec2ui32& DepthStencil::getSize() const {
		return _size;
	}

	bool DepthStencil::create(const Vec2ui32& size, DepthStencilFormat format, SampleCount sampleCount) {
		destroy();

		if (!sampleCount) sampleCount = 1;

		auto fmt = convertInternalFormat(format);
		if (fmt == DXGI_FORMAT_UNKNOWN) {
			_graphics.get<Graphics>()->error("D3D DepthStencil::create error : format error");
			return false;
		}

		D3D11_TEXTURE2D_DESC texDesc = { 0 };

		texDesc.ArraySize = 1;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.Format = fmt;
		texDesc.Width = size[0];
		texDesc.Height = size[1];
		texDesc.MipLevels = 1;
		texDesc.MiscFlags = 0;
		texDesc.SampleDesc.Count = sampleCount;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;

		auto device = _graphics.get<Graphics>()->getDevice();

		ID3D11Texture2D* tex;
		if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &tex))) {
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		memset(&dsvDesc, 0, sizeof(dsvDesc));

		dsvDesc.Format = texDesc.Format;
		if (sampleCount == 1) {
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
		} else {
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}

		if (FAILED(device->CreateDepthStencilView(tex, &dsvDesc, &_view))) {
			if (tex) tex->Release();
			return false;
		}

		tex->Release();

		_size.set(size);
		_sampleCount = sampleCount;

		return true;
	}

	void DepthStencil::destroy() {
		if (_view) {
			_view->Release();
			_view = nullptr;
		}

		_size.set(0);
		_sampleCount = 0;
	}

	DXGI_FORMAT DepthStencil::convertInternalFormat(DepthStencilFormat fmt) {
		switch (fmt) {
		case DepthStencilFormat::D16:
			return DXGI_FORMAT_D16_UNORM;
		case DepthStencilFormat::D24:
		case DepthStencilFormat::D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DepthStencilFormat::D32:
			return DXGI_FORMAT_D32_FLOAT;
		case DepthStencilFormat::D32S8:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}
}