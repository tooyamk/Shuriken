#include "BaseTextureResource.h"
#include "Graphics.h"
#include "TextureView.h"

namespace srk::modules::graphics::d3d11 {
	BaseTextureResource::BaseTextureResource(D3D11_BIND_FLAG resType) : BaseResource(resType),
		format(TextureFormat::UNKNOWN),
		internalFormat(DXGI_FORMAT_UNKNOWN),
		sampleCount(0),
		perBlockBytes(0),
		perRowBytes(0),
		arraySize(0),
		internalArraySize(0),
		mipLevels(0) {
	}

	BaseTextureResource::~BaseTextureResource() {
	}

	bool BaseTextureResource::create(Graphics& graphics, TextureType texType, const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
		TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data) {
		using namespace srk::enum_operators;

		releaseTex(graphics);

		if (sampleCount > 1 && texType != TextureType::TEX2D) {
			graphics.error("D3D Texture::create error : only support TextureType::TEX2D when sampleCount > 1");
			return false;
		}

		if (!sampleCount) sampleCount = 1;

		if (mipLevels) {
			if (sampleCount > 1) {
				graphics.error("D3D Texture::create error : could not enable multisampling and mipmap at same time");
				return _createDone(graphics, false);
			}

			auto maxLevels = TextureUtility::getMipLevels(dim.getMax());
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		} else {
			if (sampleCount > 1) {
				mipLevels = 1;
			} else {
				mipLevels = TextureUtility::getMipLevels(dim.getMax());
			}
		}

		auto isArray = arraySize && texType != TextureType::TEX3D;
		if (arraySize < 1 || texType == TextureType::TEX3D) arraySize = 1;

		auto internalFormat = Graphics::convertInternalFormat(format);
		if (internalFormat == DXGI_FORMAT_UNKNOWN) {
			graphics.error("D3D Texture::create error : not support texture fromat");
			return _createDone(graphics, false);
		}

		auto perBlockBytes = TextureUtility::getPerBlockBytes(format);
		size_t mipsBytes;
		TextureUtility::getMipsInfo(format, dim, mipLevels, &mipsBytes);

		this->size = mipsBytes * arraySize;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		if (!createInit<true>(graphics, requiredUsage & Usage::TEXTURE_RESOURCE_CREATE_ALL, preferredUsage & Usage::TEXTURE_RESOURCE_CREATE_ALL, mipLevels, cpuUsage, d3dUsage)) return _createDone(graphics, false);

		TexDesc texDesc;
		memset(&texDesc, 0, sizeof(texDesc));

		switch (texType) {
		case TextureType::TEX1D:
		{
			auto& desc = texDesc.dsec1D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
			desc.Width = dim[0];
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
			desc.Width = dim[0];
			desc.Height = dim[1];
			desc.Format = internalFormat;
			desc.MipLevels = mipLevels;
			desc.MiscFlags = 0;
			//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;//D3D11_RESOURCE_MISC_TEXTURECUBE
			desc.ArraySize = arraySize;
			desc.SampleDesc.Count = sampleCount;
			desc.SampleDesc.Quality = 0;

			break;
		}
		case TextureType::TEX3D:
		{
			auto& desc = texDesc.dsec3D;
			desc.CPUAccessFlags = cpuUsage;
			desc.Usage = d3dUsage;
			desc.BindFlags = bindType;
			desc.Width = dim[0];
			desc.Height = dim[1];
			desc.Depth = dim[2];
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
				res.SysMemPitch = dim[0] * perBlockBytes;
				res.SysMemSlicePitch = res.SysMemPitch * dim[1];
				hr = _createInternalTexture(graphics, texType, texDesc, &res);
			} else {
				std::vector<D3D11_SUBRESOURCE_DATA> resArr(mipLevels * arraySize);
				Vec2uz size2(dim.cast<2>());
				for (size_t i = 0; i < mipLevels; ++i) {
					auto& res = resArr[i];
					res.pSysMem = data[i];
					res.SysMemPitch = size2[0] * perBlockBytes;
					res.SysMemSlicePitch = res.SysMemPitch * size2[1];

					size2 = TextureUtility::getNextMipPixels(size2);

					for (size_t j = 1; j < arraySize; ++j) {
						auto idx = calcSubresource(i, j, mipLevels);
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
			graphics.error("D3D Texture::create error : internal create texture error");
			return _createDone(graphics, false);
		}

		//((ID3D11Texture2D*)_baseRes.handle)->GetDesc(&desc);

		if ((this->resUsage & Usage::MAP_READ_WRITE) != Usage::NONE) {
			mapData.resize(mipLevels * arraySize);
			Vec3uz size3(dim);
			for (decltype(mipLevels) i = 0; i < mipLevels; ++i) {
				auto& md = mapData[i];
				md.size = TextureUtility::getBlocks(format, size3).getMultiplies() * perBlockBytes;
				md.usage = Usage::NONE;

				size3 = TextureUtility::getNextMipPixels(size3);

				for (decltype(arraySize) j = 1; j < arraySize; ++j) {
					auto& md1 = mapData[calcSubresource(i, j, mipLevels)];
					md1.size = md.size;
					md1.usage = Usage::NONE;
				}
			}
		}

		this->format = format;
		this->internalFormat = internalFormat;
		this->sampleCount = sampleCount;
		this->perBlockBytes = perBlockBytes;
		this->perRowBytes = perBlockBytes * dim[0];
		this->dim.set(dim);
		internalArraySize = arraySize;
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

	Usage BaseTextureResource::map(Graphics& graphics, size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) {
			auto& md = mapData[subresource];
			return BaseResource::map(graphics, expectMapUsage, md.usage, subresource, md.res);
		}
		return Usage::NONE;
	}

	void BaseTextureResource::unmap(Graphics& graphics, size_t arraySlice, size_t mipSlice) {
		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) BaseResource::unmap(graphics, mapData[subresource].usage, subresource);
	}

	size_t BaseTextureResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		using namespace srk::enum_operators;

		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) {
			if (auto& md = mapData[subresource]; (md.usage & Usage::MAP_READ) == Usage::MAP_READ) {
				if (dst && dstLen && offset < md.size) {
					auto length = std::min(md.size - offset, dstLen);
					if (offset + length <= perRowBytes || perRowBytes == md.res.RowPitch) {
						memcpy(dst, (const uint8_t*)md.res.pData + offset, length);
					} else {
						auto out = (uint8_t*)dst;
						auto pData = (const uint8_t*)md.res.pData;
						auto len = length;

						size_t fillLen;
						if (offset) {
							auto skipRows = offset / perRowBytes;
							pData += skipRows * md.res.RowPitch;
							offset -= skipRows * perRowBytes;
							fillLen = perRowBytes - offset;
							if (fillLen > len) fillLen = len;
						} else {
							fillLen = perRowBytes;
						}

						memcpy(dst, pData + offset, fillLen);
						while (len > fillLen) {
							len -= fillLen;
							out += fillLen;
							pData += md.res.RowPitch;
							fillLen = len < perRowBytes ? len : perRowBytes;
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

	size_t BaseTextureResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		using namespace srk::enum_operators;

		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) {
			if (auto& md = mapData[subresource]; (md.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
				if (data && length && offset < md.size) {
					length = std::min(length, md.size - offset);
					if (offset + length <= perRowBytes || perRowBytes == md.res.RowPitch) {
						memcpy((uint8_t*)md.res.pData + offset, data, length);
					} else {
						auto src = (uint8_t*)data;
						auto pData = (uint8_t*)md.res.pData;
						auto len = length;

						size_t fillLen;
						if (offset) {
							auto skipRows = offset / perRowBytes;
							pData += skipRows * md.res.RowPitch;
							offset -= skipRows * perRowBytes;
							fillLen = perRowBytes - offset;
							if (fillLen > len) fillLen = len;
						} else {
							fillLen = perRowBytes;
						}

						memcpy(pData + offset, src, fillLen);
						while (len > fillLen) {
							len -= fillLen;
							src += fillLen;
							pData += md.res.RowPitch;
							fillLen = len < perRowBytes ? len : perRowBytes;
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

	bool BaseTextureResource::update(Graphics& graphics, size_t arraySlice, size_t mipSlice, const D3D11_BOX& range, const void* data) {
		using namespace srk::enum_operators;

		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && !arraySlice && mipSlice < mipLevels) {
				auto rowByteSize = (range.right - range.left) * perBlockBytes;
				graphics.getContext()->UpdateSubresource(handle, calcSubresource(mipSlice, arraySlice, mipLevels), &range, data, rowByteSize, rowByteSize * (range.bottom - range.top));
			}
			return true;
		}
		return false;
	}

	bool BaseTextureResource::copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		using namespace srk::enum_operators;
		
		if (dstArraySlice >= internalArraySize || dstMipSlice >= mipLevels || !src || src->getGraphics() != graphics) return false;
		
		auto srcBase = (BaseTextureResource*)src->getNative();
		if (srcArraySlice >= srcBase->internalArraySize || srcMipSlice >= srcBase->mipLevels) return false;

		if ((srcBase->resUsage & Usage::COPY_SRC) == Usage::NONE) {
			graphics.error("D3D Texture::copyFrom error : no Usage::COPY_SRC");
			return false;
		}

		if ((resUsage & Usage::COPY_DST) == Usage::NONE) {
			graphics.error("D3D Texture::copyFrom error : no Usage::COPY_DST");
			return false;
		}

		D3D11_BOX box;
		box.left = srcRange.pos[0];
		box.right = srcRange.pos[0] + srcRange.size[0];
		box.top = srcRange.pos[1];
		box.bottom = srcRange.pos[1] + srcRange.size[1];
		box.front = srcRange.pos[2];
		box.back = srcRange.pos[2] + srcRange.size[2];
		graphics.getContext()->CopySubresourceRegion(handle, calcSubresource(dstMipSlice, dstArraySlice, mipLevels), dstPos[0], dstPos[1], dstPos[2], srcBase->handle, calcSubresource(srcMipSlice, srcArraySlice, srcBase->mipLevels), &box);

		return true;
	}

	void BaseTextureResource::releaseTex(Graphics& graphics) {
		if (!mapData.empty()) {
			for (uint32_t i = 0, n = mapData.size(); i < n; ++i) BaseResource::unmap(graphics, mapData[i].usage, i);
			mapData.clear();
		}

		releaseRes();

		format = TextureFormat::UNKNOWN;
		internalFormat = DXGI_FORMAT_UNKNOWN;
		sampleCount = 0;
		perBlockBytes = 0;
		dim = 0;
		arraySize = 0;
		internalArraySize = 0;
		mipLevels = 0;
		mapData.clear();
	}
}