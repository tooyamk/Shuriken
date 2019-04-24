#include "Texture2D.h"
#include "math/Math.h"

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

	bool Texture2D::allocate(ui32 width, ui32 height, TextureFormat format, ui32 mipLevels, ui32 resUsage, const void* data) {
		_delTex();
		
		if (mipLevels == 0) {
			mipLevels = _calcMipLevels(width, height);
		} else if (mipLevels > 1) {
			auto maxLevels = _calcMipLevels(width, height);
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		}

		const void* texData = data;
		switch (format) {
		case TextureFormat::UNKNOWN:
			return false;
		case TextureFormat::R8G8B8:
		{
			format = TextureFormat::R8G8B8A8;

			if (data) {
				ui32 mipPixels = width * height;
				ui32 numPixels = mipPixels;
				for (ui32 i = 1; i < mipLevels; ++i) {
					mipPixels >>= 2;
					numPixels += mipPixels;
				}
				auto genData = new ui8[numPixels << 2];

				ui32 srcIdx = 0, dstIdx = 0;
				for (ui32 i = 0; i < numPixels; ++i) {
					memcpy(genData + dstIdx, (ui8*)data + srcIdx, 3);
					srcIdx += 3;
					dstIdx += 3;
					genData[dstIdx++] = 255;
				}
				texData = genData;
			}

			break;
		}
		default:
			break;
		}

		DXGI_FORMAT dxgiFmt;
		ui32 pixelSize;
		Graphics::convertDXGIFormat(format, dxgiFmt, pixelSize);

		ui32 mip0Size = width * height * pixelSize;
		ui32 mipSize = mip0Size;
		_baseRes.size = mipSize;
		for (ui32 i = 1; i < mipLevels; ++i) {
			mipSize >>= 2;
			_baseRes.size += mipSize;
		}

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
		desc.Format = dxgiFmt;
		desc.MipLevels = mipLevels;
		desc.MiscFlags = 0;
		//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;//D3D11_RESOURCE_MISC_TEXTURECUBE
		//
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		auto device = ((Graphics*)_graphics)->getDevice();

		HRESULT hr;
		if (texData) {
			if (mipLevels == 1) {
				D3D11_SUBRESOURCE_DATA res;
				res.pSysMem = texData;
				res.SysMemPitch = width * pixelSize;// Specify the size of a row in bytes.
				res.SysMemSlicePitch = 0;// As this is not a texture array or 3D texture, this parameter is ignored.
				hr = device->CreateTexture2D(&desc, &res, (ID3D11Texture2D**)&_baseRes.handle);
			} else {
				std::vector<D3D11_SUBRESOURCE_DATA> resArr(mipLevels);
				auto offset = 0;
				auto rowSize = width * pixelSize;
				ui32 mipSize = mip0Size;
				for (ui32 i = 0; i < mipLevels; ++i) {
					auto& res = resArr[i];
					res.pSysMem = (i8*)texData + offset;
					res.SysMemPitch = rowSize;
					res.SysMemSlicePitch = 0;
					
					offset += mipSize;
					mipSize >>= 2;
					rowSize >>= 1;
				}
				hr = device->CreateTexture2D(&desc, resArr.data(), (ID3D11Texture2D**)&_baseRes.handle);
			}
		} else {
			hr = device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&_baseRes.handle);
		}

		if (FAILED(hr)) {
			if (texData != data) delete[] texData;
			_delTex();
			return false;
		}

		((ID3D11Texture2D*)_baseRes.handle)->GetDesc(&desc);
		
		D3D11_SHADER_RESOURCE_VIEW_DESC vDesc;
		memset(&vDesc, 0, sizeof(vDesc));
		vDesc.Format = desc.Format;
		vDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		vDesc.Texture2D.MostDetailedMip = 0;
		vDesc.Texture2D.MipLevels = -1;
		hr = device->CreateShaderResourceView(_baseRes.handle, &vDesc, &_view);
		if (FAILED(hr)) {
			if (texData != data) delete[] texData;
			_delTex();
			return false;
		}
		((Graphics*)_graphics)->getContext()->GenerateMips(_view);
		if (texData != data) delete[] texData;

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