#include "Texture2D.h"
#include "math/Math.h"
#include <algorithm>
#include "base/Image.h"

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

	bool Texture2D::allocate(ui32 width, ui32 height, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_delTex();
		
		if (mipLevels == 0) {
			mipLevels = Image::calcMipLevels(std::max<ui32>(width, height));
		} else if (mipLevels > 1) {
			auto maxLevels = Image::calcMipLevels(std::max<ui32>(width, height));
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		}

		ByteArray autoReleaseData;
		std::vector<void*> texData(mipLevels);
		switch (format) {
		case TextureFormat::R8G8B8:
		{
			format = TextureFormat::R8G8B8A8;

			if (data) {
				auto srcPerPixelByteSize = Image::calcPerPixelByteSize(TextureFormat::R8G8B8);
				auto dstPerPixelByteSize = Image::calcPerPixelByteSize(format);
				auto dstByteSize = Image::calcMipsByteSize(width, height, mipLevels, dstPerPixelByteSize);
				auto dst = new ui8[dstByteSize];
				autoReleaseData = ByteArray((i8*)dst, dstByteSize, ByteArray::ExtMemMode::EXCLUSIVE);
				auto w = width, h = height;
				for (ui32 i = 0; i < mipLevels; ++i) {
					texData[i] = dst;
					Image::convertFormat(w, h, TextureFormat::R8G8B8, (ui8*)data[i], format, dst);
					dst += w * h;
					w = Image::calcNextMipPixelSize(w);
					h = Image::calcNextMipPixelSize(h);
				}
				data = texData.data();
			}

			break;
		}
		default:
			break;
		}

		DXGI_FORMAT dxgiFmt = Graphics::convertDXGIFormat(format);
		if (dxgiFmt == DXGI_FORMAT_UNKNOWN) return false;

		auto perPixelSize = Image::calcPerPixelByteSize(format);
		auto mipsByteSize = Image::calcMipsByteSize(width, height, mipLevels, perPixelSize);

		_baseRes.size = mipsByteSize;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		_baseRes.calcAllocateUsage(resUsage, _baseRes.size, data ? _baseRes.size : 0, cpuUsage, d3dUsage);

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
		if (data) {
			if (mipLevels == 1) {
				D3D11_SUBRESOURCE_DATA res;
				res.pSysMem = data[0];
				res.SysMemPitch = width * perPixelSize;// Specify the size of a row in bytes.
				res.SysMemSlicePitch = 0;// As this is not a texture array or 3D texture, this parameter is ignored.
				hr = device->CreateTexture2D(&desc, &res, (ID3D11Texture2D**)&_baseRes.handle);
			} else {
				std::vector<D3D11_SUBRESOURCE_DATA> resArr(mipLevels);
				auto w = width;
				for (ui32 i = 0; i < mipLevels; ++i) {
					auto& res = resArr[i];
					res.pSysMem = data[i];
					res.SysMemPitch = w * perPixelSize;
					res.SysMemSlicePitch = 0;
					
					w = Image::calcNextMipPixelSize(w);
				}
				hr = device->CreateTexture2D(&desc, resArr.data(), (ID3D11Texture2D**)&_baseRes.handle);
			}
		} else {
			hr = device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&_baseRes.handle);
		}

		if (FAILED(hr)) {
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
			_delTex();
			return false;
		}
		//((Graphics*)_graphics)->getContext()->GenerateMips(_view);

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