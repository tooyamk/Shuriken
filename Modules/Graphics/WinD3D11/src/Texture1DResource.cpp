#include "Texture1DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture1DResource::Texture1DResource(Graphics& graphics) : ITexture1DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture1DResource::~Texture1DResource() {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
	}

	TextureType Texture1DResource::getType() const {
		return TextureType::TEX1D;
	}

	const void* Texture1DResource::getNative() const {
		return &_baseTexRes;
	}

	ui32 Texture1DResource::getMipLevels() const {
		return _baseTexRes.mipLevels;
	}

	bool Texture1DResource::create(ui32 width, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
		return _baseTexRes.create(_graphics.get<Graphics>(), TextureType::TEX1D, width, 1, 1, -1, format, mipLevels, resUsage, data);
	}

	Usage Texture1DResource::map(ui32 mipLevel, Usage expectMapUsage) {
		return _baseTexRes.map(_graphics.get<Graphics>(), mipLevel, expectMapUsage);
	}

	void Texture1DResource::unmap(ui32 mipLevel) {
		_baseTexRes.unmap(_graphics.get<Graphics>(), mipLevel);
	}

	i32 Texture1DResource::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTexRes.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture1DResource::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _baseTexRes.write(mipLevel, offset, data, length);
	}

	bool Texture1DResource::write(ui32 mipLevel, ui32 left, ui32 right, const void* data) {
		D3D11_BOX box;
		box.front = 0;
		box.back = 1;
		box.top = 0;
		box.bottom = 1;
		box.left = left;
		box.right = right;

		return _baseTexRes.write(_graphics.get<Graphics>(), mipLevel, box, data);
	}
}