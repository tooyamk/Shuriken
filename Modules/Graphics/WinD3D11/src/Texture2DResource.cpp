#include "Texture2DResource.h"
#include "Graphics.h"
#include "math/Rect.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture2DResource::Texture2DResource(Graphics& graphics) : ITexture2DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture2DResource::~Texture2DResource() {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
	}

	TextureType Texture2DResource::getType() const {
		return TextureType::TEX2D;
	}

	const void* Texture2DResource::getNative() const {
		return &_baseTexRes;
	}

	ui32 Texture2DResource::getMipLevels() const {
		return _baseTexRes.mipLevels;
	}

	bool Texture2DResource::create(ui32 width, ui32 height, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
		return _baseTexRes.create(_graphics.get<Graphics>(), TextureType::TEX2D, width, height, 1, -1, format, mipLevels, resUsage, data);
	}

	Usage Texture2DResource::map(ui32 mipLevel, Usage expectMapUsage) {
		return _baseTexRes.map(_graphics.get<Graphics>(), mipLevel, expectMapUsage);
	}

	void Texture2DResource::unmap(ui32 mipLevel) {
		return _baseTexRes.unmap(_graphics.get<Graphics>(), mipLevel);
	}

	i32 Texture2DResource::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTexRes.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture2DResource::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _baseTexRes.write(mipLevel, offset, data, length);
	}

	bool Texture2DResource::write(ui32 mipLevel, const Rect<ui32>& range, const void* data) {
		D3D11_BOX box;
		box.front = 0;
		box.back = 1;
		box.top = range.top;
		box.bottom = range.bottom;
		box.left = range.left;
		box.right = range.right;

		return _baseTexRes.write(_graphics.get<Graphics>(), mipLevel, box, data);
	}
}