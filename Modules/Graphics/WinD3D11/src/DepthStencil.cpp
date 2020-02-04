#include "DepthStencil.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	DepthStencil::DepthStencil(Graphics& graphics, bool internalView) : IDepthStencil(graphics),
		_isMultisampling(false),
		_tex(nullptr),
		_view(nullptr) {
		if (_isInternal) _graphics->weakUnref();
	}

	DepthStencil::~DepthStencil() {
		if (_isInternal) _graphics.weakReset();
		_release();
	}

	const void* DepthStencil::getNative() const {
		return this;
	}

	bool DepthStencil::isMultisampling() const {
		return _isMultisampling;
	}

	const Vec2ui32& DepthStencil::getSize() const {
		return _size;
	}

	bool DepthStencil::create(const Vec2ui32& size, bool multisampling) {
		_release();

		D3D11_TEXTURE2D_DESC texDesc = { 0 };

		texDesc.ArraySize = 1;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texDesc.Width = size[0];
		texDesc.Height = size[1];
		texDesc.MipLevels = 1;
		texDesc.MiscFlags = 0;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;

		auto device = _graphics.get<Graphics>()->getDevice();

		if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &_tex))) {
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		memset(&dsvDesc, 0, sizeof(dsvDesc));

		dsvDesc.Format = texDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		if (FAILED(device->CreateDepthStencilView(_tex, &dsvDesc, &_view))) {
			return false;
		}

		_size.set(size);
		_isMultisampling = multisampling;

		return true;
	}

	void DepthStencil::_release() {
		if (_view) {
			_view->Release();
			_view = nullptr;
		}

		if (_tex) {
			_tex->Release();
			_tex = nullptr;
		}

		_size.set(0, 0);
		_isMultisampling = false;
	}
}