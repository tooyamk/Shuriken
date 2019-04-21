#include "Texture2D.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture2D::Texture2D(Graphics& graphics) : BaseResource(graphics, D3D11_BIND_SHADER_RESOURCE), ITexture2D(graphics) {
	}

	Texture2D::~Texture2D() {
		_delRes();
	}

	TextureType Texture2D::getType() const {
		return TextureType::TEX2D;
	}

	bool Texture2D::allocate(ui32 width, ui32 height, TextureFormat format, ui32 resUsage, const void* data) {
		_delRes();

		//_size = size;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		_calcAllocateUsage(resUsage, data, cpuUsage, d3dUsage);

		D3D11_TEXTURE2D_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = _bindType;
		desc.Width = width;
		desc.Height = height;
		desc.Format = Graphics::getDXGIFormat(format);
		//
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		HRESULT hr;
		if (data) {
			D3D11_SUBRESOURCE_DATA res;
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
			res.SysMemPitch = width << 2;// Specify the size of a row in bytes.
			res.SysMemSlicePitch = 0;// As this is not a texture array or 3D texture, this parameter is ignored.
			hr = _grap->getDevice()->CreateTexture2D(&desc, &res, (ID3D11Texture2D**)&_handle);
		} else {
			hr = _grap->getDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&_handle);
		}
		if (FAILED(hr)) {
			_delRes();
			return false;
		}

		return true;
	}
}