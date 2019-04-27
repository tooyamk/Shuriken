#include "BaseTexture.h"
#include "base/Image.h"

namespace aurora::modules::graphics::win_d3d11 {
	BaseTexture::BaseTexture(UINT resType) : BaseResource(resType),
		format(TextureFormat::UNKNOWN),
		perPixelSize(0),
		width(0),
		height(0),
		depth(0),
		arraySize(0),
		mipLevels(0),
		view(nullptr) {
	}

	BaseTexture::~BaseTexture() {
	}

	bool BaseTexture::allocate(Graphics* graphics, TextureType texType, ui32 width, ui32 height, ui32 depth, i32 arraySize, 
		TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		releaseTex(graphics);

		if (mipLevels == 0) {
			mipLevels = Image::calcMipLevels(std::max<ui32>(std::max<ui32>(width, height), depth));
		} else if (mipLevels > 1) {
			auto maxLevels = Image::calcMipLevels(std::max<ui32>(std::max<ui32>(width, height), depth));
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		}

		ui32 arrayCount = arraySize < 0 ? 1 : arraySize;

		ByteArray autoReleaseData;
		std::vector<void*> texData(mipLevels);
		switch (format) {
		case TextureFormat::R8G8B8:
		{
			format = TextureFormat::R8G8B8A8;

			if (data) {
				auto srcPerPixelByteSize = Image::calcPerPixelByteSize(TextureFormat::R8G8B8);
				auto dstPerPixelByteSize = Image::calcPerPixelByteSize(format);
				auto dstByteSize = Image::calcMipsByteSize(width, height, depth, mipLevels, dstPerPixelByteSize);
				auto dst = new ui8[dstByteSize];
				autoReleaseData = ByteArray((i8*)dst, dstByteSize, ByteArray::ExtMemMode::EXCLUSIVE);
				auto w = width, h = height, d = depth;
				for (ui32 i = 0; i < mipLevels; ++i) {
					texData[i] = dst;
					auto src = (ui8*)data[i];
					auto numPixels = w * h;
					auto srcPitch = numPixels * srcPerPixelByteSize;
					auto dstPitch = numPixels * dstPerPixelByteSize;
					for (ui32 j = 0; j < d; ++j) {
						Image::convertFormat(w, h, TextureFormat::R8G8B8, src, format, dst);
						src += srcPitch;
						dst += dstPitch;
					}
					w = Image::calcNextMipPixelSize(w);
					h = Image::calcNextMipPixelSize(h);
					d = Image::calcNextMipPixelSize(d);
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
		auto mipsByteSize = Image::calcMipsByteSize(width, height, depth, mipLevels, perPixelSize);

		this->size = mipsByteSize * arrayCount;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		calcAllocateUsage(resUsage, this->size, data ? this->size : 0, mipLevels, cpuUsage, d3dUsage);

		TexDesc texDesc;
		memset(&texDesc, 0, sizeof(texDesc));

		switch (texType) {
		case TextureType::TEX1D:
		{
			auto& desc = texDesc.dsec1D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
			desc.Width = width;
			desc.Format = dxgiFmt;
			desc.MipLevels = mipLevels;
			desc.MiscFlags = 0;
			//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;//D3D11_RESOURCE_MISC_TEXTURECUBE
			//
			desc.ArraySize = 1;

			break;
		}
		case TextureType::TEX2D:
		{
			auto& desc = texDesc.dsec2D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
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

			break;
		}
		case TextureType::TEX3D:
		{
			auto& desc = texDesc.dsec3D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
			desc.Width = width;
			desc.Height = height;
			desc.Depth = depth;
			desc.Format = dxgiFmt;
			desc.MipLevels = mipLevels;
			desc.MiscFlags = 0;

			break;
		}
		default:
			break;
		}

		HRESULT hr;
		if (false) {
			if (mipLevels == 1) {
				D3D11_SUBRESOURCE_DATA res;
				res.pSysMem = data[0];
				res.SysMemPitch = width * perPixelSize;
				res.SysMemSlicePitch = res.SysMemPitch * height;
				hr = create(graphics, texType, texDesc, &res);
			} else {
				std::vector<D3D11_SUBRESOURCE_DATA> resArr(mipLevels);
				auto w = width, h = height;
				for (ui32 i = 0; i < mipLevels; ++i) {
					auto& res = resArr[i];
					res.pSysMem = data[i];
					res.SysMemPitch = w * perPixelSize;
					res.SysMemSlicePitch = res.SysMemPitch * h;

					w = Image::calcNextMipPixelSize(w);
					h = Image::calcNextMipPixelSize(h);
				}
				hr = create(graphics, texType, texDesc, resArr.data());
			}
		} else {
			hr = create(graphics, texType, texDesc, nullptr);
		}

		if (FAILED(hr)) {
			releaseTex(graphics);
			return false;
		}

		//((ID3D11Texture2D*)_baseRes.handle)->GetDesc(&desc);

		if (bindType) {
			D3D11_SHADER_RESOURCE_VIEW_DESC vDesc;
			memset(&vDesc, 0, sizeof(vDesc));
			vDesc.Format = dxgiFmt;

			switch (texType) {
			case TextureType::TEX1D:
			{
				vDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
				vDesc.Texture1D.MostDetailedMip = 0;
				vDesc.Texture1D.MipLevels = -1;

				break;
			}
			case TextureType::TEX2D:
			{
				vDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				vDesc.Texture2D.MostDetailedMip = 0;
				vDesc.Texture2D.MipLevels = -1;
				//vDesc.Texture2DArray.ArraySize = 0;
				//vDesc.Texture2DArray.FirstArraySlice = 0;
				//vDesc.Texture2DArray.MostDetailedMip = 0;
				//vDesc.Texture2DArray.MipLevels = -1;

				break;
			}
			case TextureType::TEX3D:
			{
				vDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				vDesc.Texture3D.MostDetailedMip = 0;
				vDesc.Texture3D.MipLevels = -1;

				break;
			}
			default:
				break;
			}

			hr = graphics->getDevice()->CreateShaderResourceView(handle, &vDesc, &view);
			if (FAILED(hr)) {
				releaseTex(graphics);
				return false;
			}
		}

		if ((this->resUsage & Usage::CPU_READ_WRITE) != Usage::NONE) {
			mappedRes.resize(mipLevels);
			ui32 w = width, h = height, d = depth;
			for (ui32 i = 0; i < mipLevels; ++i) {
				auto& mapped = mappedRes[i];
				mapped.size = w * h * d * perPixelSize;
				mapped.usage = Usage::NONE;
				w = Image::calcNextMipPixelSize(w);
				h = Image::calcNextMipPixelSize(h);
				d = Image::calcNextMipPixelSize(d);
			}
		}

		this->format = format;
		this->perPixelSize = perPixelSize;
		this->width = width;
		this->height = height;
		this->depth = depth;
		this->arraySize = arraySize;
		this->mipLevels = mipLevels;
		//((Graphics*)_graphics)->getContext()->GenerateMips(_view);

		return true;
	}

	HRESULT BaseTexture::create(Graphics* graphics, TextureType texType, const TexDesc& desc, const D3D11_SUBRESOURCE_DATA* pInitialData) {
		switch (texType) {
		case TextureType::TEX1D:
			return graphics->getDevice()->CreateTexture1D(&desc.dsec1D, pInitialData, (ID3D11Texture1D**)&handle);
		case TextureType::TEX2D:
			return graphics->getDevice()->CreateTexture2D(&desc.dsec2D, pInitialData, (ID3D11Texture2D**)&handle);
		case TextureType::TEX3D:
			return graphics->getDevice()->CreateTexture3D(&desc.dsec3D, pInitialData, (ID3D11Texture3D**)&handle);
		default:
			return -1;
		}
	}

	Usage BaseTexture::map(Graphics* graphics, ui32 mipLevel, Usage expectMapUsage) {
		if (mipLevel < mappedRes.size()) {
			auto& mapped = mappedRes[mipLevel];
			return BaseResource::map(graphics, expectMapUsage, mapped.usage, mipLevel, mapped.res);
		}
		return Usage::NONE;
	}

	void BaseTexture::unmap(Graphics* graphics, ui32 mipLevel) {
		if (mipLevel < mappedRes.size()) BaseResource::unmap(graphics, mappedRes[mipLevel].usage, mipLevel);
	}

	i32 BaseTexture::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		if (mipLevel < mappedRes.size()) {
			auto& mapped = mappedRes[mipLevel];
			if ((mapped.usage & Usage::CPU_READ) == Usage::CPU_READ) {
				if (dst && dstLen && readLen && offset < mapped.size) {
					if (readLen < 0) readLen = mapped.size - offset;
					if ((ui32)readLen > dstLen) readLen = dstLen;
					memcpy(dst, (i8*)mapped.res.pData + offset, readLen);
					return readLen;
				}
				return 0;
			}
		}
		return -1;
	}

	i32 BaseTexture::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		if (mipLevel < mappedRes.size()) {
			auto& mapped = mappedRes[mipLevel];
			if ((mapped.usage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
				if (data && length && offset < mapped.size) {
					length = std::min<ui32>(length, mapped.size - offset);
					memcpy((i8*)mapped.res.pData + offset, data, length);
					return length;
				}
				return 0;
			}
		}
		return -1;
	}

	bool BaseTexture::write(Graphics* graphics, ui32 mipLevel, const D3D11_BOX& range, const void* data) {
		if ((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
			if (data && mipLevel < mipLevels) {
				ui32 w = width, h = height;
				Image::calcSpecificMipPixelSize(w, h, mipLevel);
				auto rowByteSize = w * perPixelSize;

				graphics->getContext()->UpdateSubresource(handle, mipLevel, &range, data, rowByteSize, rowByteSize * h);
			}
			return true;
		}
		return false;
	}

	void BaseTexture::releaseTex(Graphics* graphics) {
		if (!mappedRes.empty()) {
			for (ui32 i = 0, n = mappedRes.size(); i < n; ++i) BaseResource::unmap(graphics, mappedRes[i].usage, i);
			mappedRes.clear();
		}

		if (view) {
			view->Release();
			view = nullptr;
		}

		releaseRes(graphics);
		format = TextureFormat::UNKNOWN;
		perPixelSize = 0;
		width = 0;
		height = 0;
		depth = 0;
		arraySize = 0;
		mipLevels = 0;
		mappedRes.clear();
	}
}