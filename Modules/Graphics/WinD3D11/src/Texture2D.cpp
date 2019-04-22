#include "Texture2D.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture2D::Texture2D(Graphics& graphics) : ITexture2D(graphics),
		_baseRes(D3D11_BIND_SHADER_RESOURCE),
		_view(nullptr) {
	}

	Texture2D::~Texture2D() {
		_delTex();
	}

	TextureType Texture2D::getType() const {
		return TextureType::TEX2D;
	}

	bool Texture2D::allocate(ui32 width, ui32 height, TextureFormat format, ui32 resUsage, const void* data) {
		_delTex();

		_baseRes.size = 4;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		_baseRes.calcAllocateUsage(resUsage, data, cpuUsage, d3dUsage);

		D3D11_TEXTURE2D_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = _baseRes._bindType;
		desc.Width = width;
		desc.Height = height;
		desc.Format = Graphics::getDXGIFormat(format);
		//
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		auto device = ((Graphics*)_graphics)->getDevice();

		HRESULT hr;
		if (data) {
			D3D11_SUBRESOURCE_DATA res;
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
			res.SysMemPitch = width << 2;// Specify the size of a row in bytes.
			res.SysMemSlicePitch = 0;// As this is not a texture array or 3D texture, this parameter is ignored.
			hr = device->CreateTexture2D(&desc, &res, (ID3D11Texture2D**)&_baseRes.handle);
		} else {
			hr = device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&_baseRes.handle);
		}

		if (FAILED(hr)) {
			_delTex();
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC vDesc;
		memset(&vDesc, 0, sizeof(vDesc));
		vDesc.Format = desc.Format;
		vDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		vDesc.Texture2D.MipLevels = 1;
		vDesc.Texture2D.MostDetailedMip = 0;
		hr = device->CreateShaderResourceView(_baseRes.handle, &vDesc, &_view);
		if (FAILED(hr)) {
			_delTex();
			return false;
		}

		return true;
	}

	void Texture2D::_delTex() {
		if (_view) {
			_view->Release();
			_view = nullptr;
		}

		_baseRes.releaseRes((Graphics*)_graphics);
	}
}