#include "Texture3D.h"
#include "base/Image.h"
#include "math/Box.h"
#include "math/Math.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	Texture3D::Texture3D(Graphics& graphics) : ITexture2D(graphics),
		_format(TextureFormat::UNKNOWN),
		_perPixelSize(0),
		_width(0),
		_height(0),
		_mipLevels(0),
		_baseRes(D3D11_BIND_SHADER_RESOURCE),
		_view(nullptr) {
	}

	Texture3D::~Texture3D() {
		_releaseTex();
	}

	TextureType Texture3D::getType() const {
		return TextureType::TEX3D;
	}

	bool Texture3D::allocate(ui32 width, ui32 height, ui32 depth, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_releaseTex();

		if (mipLevels == 0) {
			mipLevels = Image::calcMipLevels(std::max<ui32>(std::max<ui32>(width, height), depth));
		} else if (mipLevels > 1) {
			auto maxLevels = Image::calcMipLevels(std::max<ui32>(std::max<ui32>(width, height), depth));
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

		_baseRes.size = mipsByteSize;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		_baseRes.calcAllocateUsage(resUsage, _baseRes.size, data ? _baseRes.size : 0, mipLevels, cpuUsage, d3dUsage);

		D3D11_TEXTURE3D_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = _baseRes.bindType;
		desc.Width = width;
		desc.Height = height;
		desc.Depth = depth;
		desc.Format = dxgiFmt;
		desc.MipLevels = mipLevels;
		desc.MiscFlags = 0;

		auto device = ((Graphics*)_graphics)->getDevice();

		HRESULT hr;
		if (data) {
			if (mipLevels == 1) {
				D3D11_SUBRESOURCE_DATA res;
				res.pSysMem = data[0];
				res.SysMemPitch = width * perPixelSize;
				res.SysMemSlicePitch = res.SysMemPitch * height;
				hr = device->CreateTexture3D(&desc, &res, (ID3D11Texture3D**)&_baseRes.handle);
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
				hr = device->CreateTexture3D(&desc, resArr.data(), (ID3D11Texture3D**)&_baseRes.handle);
			}
		} else {
			hr = device->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**)&_baseRes.handle);
		}

		if (FAILED(hr)) {
			_releaseTex();
			return false;
		}

		//((ID3D11Texture2D*)_baseRes.handle)->GetDesc(&desc);

		if (desc.BindFlags) {
			D3D11_SHADER_RESOURCE_VIEW_DESC vDesc;
			memset(&vDesc, 0, sizeof(vDesc));
			vDesc.Format = desc.Format;
			vDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			vDesc.Texture3D.MostDetailedMip = 0;
			vDesc.Texture3D.MipLevels = -1;
			hr = device->CreateShaderResourceView(_baseRes.handle, &vDesc, &_view);
			if (FAILED(hr)) {
				_releaseTex();
				return false;
			}
		}

		if ((_baseRes.resUsage & Usage::CPU_READ_WRITE) != Usage::NONE) {
			_mappedRes.resize(mipLevels);
			ui32 w = width, h = height, d = depth;
			for (ui32 i = 0; i < mipLevels; ++i) {
				auto& mapped = _mappedRes[i];
				mapped.size = w * h * d * perPixelSize;
				mapped.usage = Usage::NONE;
				w = Image::calcNextMipPixelSize(w);
				h = Image::calcNextMipPixelSize(h);
				d = Image::calcNextMipPixelSize(d);
			}
		}

		_format = format;
		_perPixelSize = perPixelSize;
		_width = width;
		_height = height;
		_depth = depth;
		_mipLevels = mipLevels;
		//((Graphics*)_graphics)->getContext()->GenerateMips(_view);

		return true;
	}

	Usage Texture3D::map(ui32 mipLevel, Usage mapUsage) {
		if (mipLevel < _mappedRes.size()) {
			auto& mapped = _mappedRes[mipLevel];
			return _baseRes.map((Graphics*)_graphics, mapUsage, mapped.usage, mipLevel, mapped.res);
		}
		return Usage::NONE;
	}

	void Texture3D::unmap(ui32 mipLevel) {
		if (mipLevel < _mappedRes.size()) _baseRes.unmap((Graphics*)_graphics, _mappedRes[mipLevel].usage, mipLevel);
	}

	i32 Texture3D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		if (mipLevel < _mappedRes.size()) {
			auto& mapped = _mappedRes[mipLevel];
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

	i32 Texture3D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		if (mipLevel < _mappedRes.size()) {
			auto& mapped = _mappedRes[mipLevel];
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

	bool Texture3D::write(ui32 mipLevel, const Box<ui32>& range, const void* data) {
		if ((_baseRes.resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
			if (data && mipLevel < _mipLevels) {
				ui32 w = _width, h = _height;
				Image::calcSpecificMipPixelSize(w, h, mipLevel);
				auto rowByteSize = w * _perPixelSize;

				D3D11_BOX box;
				box.back = range.back;
				box.front = range.front;
				box.top = range.top;
				box.bottom = range.bottom;
				box.left = range.left;
				box.right = range.right;
				((Graphics*)_graphics)->getContext()->UpdateSubresource(_baseRes.handle, mipLevel, &box, data, rowByteSize, rowByteSize * h);
			}
			return true;
		}
		return false;
	}

	void Texture3D::_releaseTex() {
		if (!_mappedRes.empty()) {
			for (ui32 i = 0, n = _mappedRes.size(); i < n; ++i) _baseRes.unmap((Graphics*)_graphics, _mappedRes[i].usage, i);
			_mappedRes.clear();
		}

		if (_view) {
			_view->Release();
			_view = nullptr;
		}

		_baseRes.releaseRes((Graphics*)_graphics);
		_format = TextureFormat::UNKNOWN;
		_perPixelSize = 0;
		_width = 0;
		_height = 0;
		_depth = 0;
		_mipLevels = 0;
		_mappedRes.clear();
	}
}