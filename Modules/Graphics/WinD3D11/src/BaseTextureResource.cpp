#include "BaseTextureResource.h"
#include "Graphics.h"
#include "TextureView.h"
#include "base/Image.h"

namespace aurora::modules::graphics::win_d3d11 {
	BaseTextureResource::BaseTextureResource(UINT resType) : BaseResource(resType),
		format(TextureFormat::UNKNOWN),
		internalFormat(DXGI_FORMAT_UNKNOWN),
		perPixelSize(0),
		arraySize(0),
		mipLevels(0) {
	}

	BaseTextureResource::~BaseTextureResource() {
	}

	bool BaseTextureResource::create(Graphics& graphics, TextureType texType, const Vec3ui32& size, ui32 arraySize, ui32 mipLevels,
		TextureFormat format, Usage resUsage, const void*const* data) {
		releaseTex(graphics);

		if (mipLevels == 0) {
			mipLevels = Image::calcMipLevels(size.getMax());
		} else if (mipLevels > 1) {
			auto maxLevels = Image::calcMipLevels(size.getMax());
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		}

		auto isArray = arraySize && texType != TextureType::TEX3D;
		if (arraySize < 1 || texType == TextureType::TEX3D) arraySize = 1;

		ByteArray autoReleaseData;
		std::vector<void*> texData(mipLevels * arraySize);
		switch (format) {
		case TextureFormat::R8G8B8:
		{
			format = TextureFormat::R8G8B8A8;

			if (data) {
				auto srcPerPixelByteSize = Image::calcPerPixelByteSize(TextureFormat::R8G8B8);
				auto dstPerPixelByteSize = Image::calcPerPixelByteSize(format);
				auto dstByteSize = Image::calcMipsByteSize(size, mipLevels, dstPerPixelByteSize) * arraySize;
				auto dst = new ui8[dstByteSize];
				autoReleaseData = ByteArray((i8*)dst, dstByteSize, ByteArray::ExtMemMode::EXCLUSIVE);
				Vec3ui32 size3;
				for (ui32 i = 0; i < arraySize; ++i) {
					size3.set(size);
					for (ui32 j = 0; j < mipLevels; ++j) {
						texData[j] = dst;
						auto src = (ui8*)data[j];
						auto numPixels = size3[0] * size3[1];
						auto srcPitch = numPixels * srcPerPixelByteSize;
						auto dstPitch = numPixels * dstPerPixelByteSize;
						for (ui32 k = 0; k < size3[2]; ++k) {
							Image::convertFormat((Vec2ui32&)size, TextureFormat::R8G8B8, src, format, dst);
							src += srcPitch;
							dst += dstPitch;
						}
						Image::calcNextMipPixelSize(size3);
					}
				}
				data = texData.data();
			}

			break;
		}
		default:
			break;
		}

		DXGI_FORMAT internalFormat = Graphics::convertInternalFormat(format);
		if (internalFormat == DXGI_FORMAT_UNKNOWN) return _createDone(false);

		auto perPixelSize = Image::calcPerPixelByteSize(format);
		auto mipsByteSize = Image::calcMipsByteSize(size, mipLevels, perPixelSize);

		this->size = mipsByteSize * arraySize;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		createInit(resUsage, this->size, data ? this->size : 0, mipLevels, cpuUsage, d3dUsage);

		TexDesc texDesc;
		memset(&texDesc, 0, sizeof(texDesc));

		switch (texType) {
		case TextureType::TEX1D:
		{
			auto& desc = texDesc.dsec1D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
			desc.Width = size[0];
			desc.Format = internalFormat;
			desc.MipLevels = mipLevels;
			desc.MiscFlags = 0;
			//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;//D3D11_RESOURCE_MISC_TEXTURECUBE
			//
			desc.ArraySize = arraySize;

			break;
		}
		case TextureType::TEX2D:
		{
			auto& desc = texDesc.dsec2D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
			desc.Width = size[0];
			desc.Height = size[1];
			desc.Format = internalFormat;
			desc.MipLevels = mipLevels;
			desc.MiscFlags = 0;
			//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;//D3D11_RESOURCE_MISC_TEXTURECUBE
			//
			desc.ArraySize = arraySize;
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
			desc.Width = size[0];
			desc.Height = size[1];
			desc.Depth = size[2];
			desc.Format = internalFormat;
			desc.MipLevels = mipLevels;
			desc.MiscFlags = 0;

			break;
		}
		default:
			break;
		}

		HRESULT hr;
		if (data) {
			if (mipLevels == 1) {
				D3D11_SUBRESOURCE_DATA res;
				res.pSysMem = data[0];
				res.SysMemPitch = size[0] * perPixelSize;
				res.SysMemSlicePitch = res.SysMemPitch * size[1];
				hr = _createInternalTexture(graphics, texType, texDesc, &res);
			} else {
				std::vector<D3D11_SUBRESOURCE_DATA> resArr(mipLevels * arraySize);
				Vec2ui32 size2(size.slice<2>());
				for (ui32 i = 0; i < mipLevels; ++i) {
					auto& res = resArr[i];
					res.pSysMem = data[i];
					res.SysMemPitch = size2[0] * perPixelSize;
					res.SysMemSlicePitch = res.SysMemPitch * size2[1];

					Image::calcNextMipPixelSize(size2);

					for (ui32 j = 1; j < arraySize; ++j) {
						auto idx = i + j * mipLevels;
						auto& res1 = resArr[idx];
						res1.pSysMem = data[idx];
						res1.SysMemPitch = res.SysMemPitch;
						res1.SysMemSlicePitch = res.SysMemSlicePitch;
					}
				}
				hr = _createInternalTexture(graphics, texType, texDesc, resArr.data());
			}
		} else {
			hr = _createInternalTexture(graphics, texType, texDesc, nullptr);
		}

		if (FAILED(hr)) {
			releaseTex(graphics);
			return _createDone(false);
		}

		//((ID3D11Texture2D*)_baseRes.handle)->GetDesc(&desc);

		if ((this->resUsage & Usage::MAP_READ_WRITE) != Usage::NONE) {
			mappedRes.resize(mipLevels * arraySize);
			Vec3ui32 size3(size);
			for (ui32 i = 0; i < mipLevels; ++i) {
				auto& mapped = mappedRes[i];
				mapped.size = size3.getMultiplies()* perPixelSize;
				mapped.usage = Usage::NONE;

				Image::calcNextMipPixelSize(size3);

				for (ui32 j = 1; j < arraySize; ++j) {
					auto& mapped1 = mappedRes[i + j * arraySize];
					mapped1.size = mapped.size;
					mapped1.usage = Usage::NONE;
				}
			}
		}

		this->format = format;
		this->internalFormat = internalFormat;
		this->perPixelSize = perPixelSize;
		texSize.set(size);
		this->arraySize = isArray ? arraySize : 0;
		this->mipLevels = mipLevels;
		//((Graphics*)_graphics)->getContext()->GenerateMips(_view);

		return _createDone(true);
	}

	bool BaseTextureResource::_createDone(bool succeeded) {
		for (auto& itr : views) itr->onResRecreated();
		return succeeded;
	}

	HRESULT BaseTextureResource::_createInternalTexture(Graphics& graphics, TextureType texType, const BaseTextureResource::TexDesc& desc, const D3D11_SUBRESOURCE_DATA* pInitialData) {
		switch (texType) {
		case TextureType::TEX1D:
			return graphics.getDevice()->CreateTexture1D(&desc.dsec1D, pInitialData, (ID3D11Texture1D**)&handle);
		case TextureType::TEX2D:
			return graphics.getDevice()->CreateTexture2D(&desc.dsec2D, pInitialData, (ID3D11Texture2D**)&handle);
		case TextureType::TEX3D:
			return graphics.getDevice()->CreateTexture3D(&desc.dsec3D, pInitialData, (ID3D11Texture3D**)&handle);
		default:
			return -1;
		}
	}

	Usage BaseTextureResource::map(Graphics& graphics, ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) {
		if (ui32 subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) {
			auto& mapped = mappedRes[subresource];
			return BaseResource::map(graphics, expectMapUsage, mapped.usage, subresource, mapped.res);
		}
		return Usage::NONE;
	}

	void BaseTextureResource::unmap(Graphics& graphics, ui32 arraySlice, ui32 mipSlice) {
		if (ui32 subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) BaseResource::unmap(graphics, mappedRes[subresource].usage, subresource);
	}

	ui32 BaseTextureResource::read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen) {
		if (ui32 subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) {
			if (auto& mapped = mappedRes[subresource]; (mapped.usage & Usage::MAP_READ) == Usage::MAP_READ) {
				if (dst && dstLen && offset < mapped.size) {
					auto readLen = std::min<ui32>(mapped.size - offset, dstLen);
					memcpy(dst, (i8*)mapped.res.pData + offset, readLen);
					return readLen;
				}
				return 0;
			}
		}
		return -1;
	}

	ui32 BaseTextureResource::write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) {
		if (ui32 subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) {
			if (auto& mapped = mappedRes[subresource]; (mapped.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
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

	bool BaseTextureResource::update(Graphics& graphics, ui32 arraySlice, ui32 mipSlice, const D3D11_BOX& range, const void* data) {
		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && !arraySlice && mipSlice < mipLevels) {
				Vec2ui32 size(texSize.slice<2>());
				Image::calcSpecificMipPixelSize(size, mipSlice);
				auto rowByteSize = size[0] * perPixelSize;

				graphics.getContext()->UpdateSubresource(handle, mipSlice, &range, data, rowByteSize, rowByteSize * size[1]);
			}
			return true;
		}
		return false;
	}

	void BaseTextureResource::releaseTex(Graphics& graphics) {
		if (!mappedRes.empty()) {
			for (ui32 i = 0, n = mappedRes.size(); i < n; ++i) BaseResource::unmap(graphics, mappedRes[i].usage, i);
			mappedRes.clear();
		}

		releaseRes();

		format = TextureFormat::UNKNOWN;
		internalFormat = DXGI_FORMAT_UNKNOWN,
		perPixelSize = 0;
		texSize.set(0);
		arraySize = 0;
		mipLevels = 0;
		mappedRes.clear();
	}

	void BaseTextureResource::addView(TextureView& view) {
		if (auto itr = views.find(&view); itr == views.end()) views.emplace(&view);
	}

	void BaseTextureResource::removeView(TextureView& view) {
		if (auto itr = views.find(&view); itr != views.end()) views.erase(itr);
	}
}