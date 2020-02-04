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

	bool BaseTextureResource::create(Graphics& graphics, TextureType texType, const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels,
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
				auto dst = new uint8_t[dstByteSize];
				autoReleaseData = ByteArray(dst, dstByteSize, ByteArray::Usage::EXCLUSIVE);
				Vec3ui32 size3;
				for (uint32_t i = 0; i < arraySize; ++i) {
					size3.set(size);
					for (uint32_t j = 0; j < mipLevels; ++j) {
						texData[j] = dst;
						auto src = (uint8_t*)data[j];
						auto numPixels = size3[0] * size3[1];
						auto srcPitch = numPixels * srcPerPixelByteSize;
						auto dstPitch = numPixels * dstPerPixelByteSize;
						for (uint32_t k = 0; k < size3[2]; ++k) {
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
		if (internalFormat == DXGI_FORMAT_UNKNOWN) {
			graphics.error("d3d11 Texture::create error : not support fromat");
			return _createDone(graphics, false);
		}

		auto perPixelSize = Image::calcPerPixelByteSize(format);
		auto mipsByteSize = Image::calcMipsByteSize(size, mipLevels, perPixelSize);

		this->size = mipsByteSize * arraySize;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		if (!createInit<true>(graphics, resUsage, this->size, data ? this->size : 0, mipLevels, cpuUsage, d3dUsage)) return _createDone(graphics, false);

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
				for (uint32_t i = 0; i < mipLevels; ++i) {
					auto& res = resArr[i];
					res.pSysMem = data[i];
					res.SysMemPitch = size2[0] * perPixelSize;
					res.SysMemSlicePitch = res.SysMemPitch * size2[1];

					Image::calcNextMipPixelSize(size2);

					for (uint32_t j = 1; j < arraySize; ++j) {
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
			graphics.error("d3d11 Texture::create error : internal create texture error");
			return _createDone(graphics, false);
		}

		//((ID3D11Texture2D*)_baseRes.handle)->GetDesc(&desc);

		if ((this->resUsage & Usage::MAP_READ_WRITE) != Usage::NONE) {
			mappedRes.resize(mipLevels * arraySize);
			Vec3ui32 size3(size);
			for (uint32_t i = 0; i < mipLevels; ++i) {
				auto& mapped = mappedRes[i];
				mapped.size = size3.getMultiplies() * perPixelSize;
				mapped.usage = Usage::NONE;

				Image::calcNextMipPixelSize(size3);

				for (uint32_t j = 1; j < arraySize; ++j) {
					auto& mapped1 = mappedRes[i + j * arraySize];
					mapped1.size = mapped.size;
					mapped1.usage = Usage::NONE;
				}
			}
		}

		this->format = format;
		this->internalFormat = internalFormat;
		this->perPixelSize = perPixelSize;
		this->perRowPixelSize = perPixelSize * size[0];
		texSize.set(size);
		this->arraySize = isArray ? arraySize : 0;
		this->mipLevels = mipLevels;
		//((Graphics*)_graphics)->getContext()->GenerateMips(_view);

		return _createDone(graphics, true);
	}

	bool BaseTextureResource::_createDone(Graphics& graphics, bool succeeded) {
		if (!succeeded) releaseTex(graphics);
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

	Usage BaseTextureResource::map(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
		if  (uint32_t subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) {
			auto& mapped = mappedRes[subresource];
			return BaseResource::map(graphics, expectMapUsage, mapped.usage, subresource, mapped.res);
		}
		return Usage::NONE;
	}

	void BaseTextureResource::unmap(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice) {
		if  (uint32_t subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) BaseResource::unmap(graphics, mappedRes[subresource].usage, subresource);
	}

	uint32_t BaseTextureResource::read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		if  (uint32_t subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) {
			if (auto& mapped = mappedRes[subresource]; (mapped.usage & Usage::MAP_READ) == Usage::MAP_READ) {
				if (dst && dstLen && offset < mapped.size) {
					auto length = std::min<uint32_t>(mapped.size - offset, dstLen);
					if (offset + length <= perRowPixelSize || perRowPixelSize == mapped.res.RowPitch) {
						memcpy(dst, (uint8_t*)mapped.res.pData + offset, length);
					} else {
						auto out = (uint8_t*)dst;
						auto pData = (uint8_t*)mapped.res.pData;
						auto len = length;

						uint32_t fillLen;
						if (offset) {
							auto skipRows = offset / perRowPixelSize;
							pData += skipRows * mapped.res.RowPitch;
							offset -= skipRows * perRowPixelSize;
							fillLen = perRowPixelSize - offset;
							if (fillLen > len) fillLen = len;
						} else {
							fillLen = perRowPixelSize;
						}

						memcpy(dst, pData + offset, fillLen);
						while (len > fillLen) {
							len -= fillLen;
							out += fillLen;
							pData += mapped.res.RowPitch;
							fillLen = len < perRowPixelSize ? len : perRowPixelSize;
							memcpy(out, pData, fillLen);
						}
					}
					return length;
				}
				return 0;
			}
		}
		return -1;
	}

	uint32_t BaseTextureResource::write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		if  (uint32_t subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mappedRes.size()) {
			if (auto& mapped = mappedRes[subresource]; (mapped.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
				if (data && length && offset < mapped.size) {
					length = std::min<uint32_t>(length, mapped.size - offset);
					if (offset + length <= perRowPixelSize || perRowPixelSize == mapped.res.RowPitch) {
						memcpy((uint8_t*)mapped.res.pData + offset, data, length);
					} else {
						auto src = (uint8_t*)data;
						auto pData = (uint8_t*)mapped.res.pData;
						auto len = length;

						uint32_t fillLen;
						if (offset) {
							auto skipRows = offset / perRowPixelSize;
							pData += skipRows * mapped.res.RowPitch;
							offset -= skipRows * perRowPixelSize;
							fillLen = perRowPixelSize - offset;
							if (fillLen > len) fillLen = len;
						} else {
							fillLen = perRowPixelSize;
						}

						memcpy(pData + offset, src, fillLen);
						while (len > fillLen) {
							len -= fillLen;
							src += fillLen;
							pData += mapped.res.RowPitch;
							fillLen = len < perRowPixelSize ? len : perRowPixelSize;
							memcpy(pData, src, fillLen);
						}
					}
					return length;
				}
				return 0;
			}
		}
		return -1;
	}

	bool BaseTextureResource::update(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, const D3D11_BOX& range, const void* data) {
		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && !arraySlice && mipSlice < mipLevels) {
				auto rowByteSize = (range.right - range.left) * perPixelSize;
				graphics.getContext()->UpdateSubresource(handle, mipSlice, &range, data, rowByteSize, rowByteSize * (range.bottom - range.top));
			}
			return true;
		}
		return false;
	}

	void BaseTextureResource::releaseTex(Graphics& graphics) {
		if (!mappedRes.empty()) {
			for (uint32_t i = 0, n = mappedRes.size(); i < n; ++i) BaseResource::unmap(graphics, mappedRes[i].usage, i);
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
}