#include "Texture3DResource.h"
#include "Graphics.h"
#include "math/Box.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture3DResource::Texture3DResource(Graphics& graphics) : ITexture3DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture3DResource::~Texture3DResource() {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
	}

	TextureType Texture3DResource::getType() const {
		return TextureType::TEX3D;
	}

	const void* Texture3DResource::getNative() const {
		return &_baseTexRes;
	}

	ui32 Texture3DResource::getMipLevels() const {
		return _baseTexRes.mipLevels;
	}

	bool Texture3DResource::create(ui32 width, ui32 height, ui32 depth, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
		return _baseTexRes.create(_graphics.get<Graphics>(), TextureType::TEX3D, width, height, depth, -1, format, mipLevels, resUsage, data);
	}

	Usage Texture3DResource::map(ui32 mipLevel, Usage expectMapUsage) {
		return _baseTexRes.map(_graphics.get<Graphics>(), mipLevel, expectMapUsage);
	}

	void Texture3DResource::unmap(ui32 mipLevel) {
		return _baseTexRes.unmap(_graphics.get<Graphics>(), mipLevel);
	}

	i32 Texture3DResource::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTexRes.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture3DResource::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _baseTexRes.write(mipLevel, offset, data, length);
	}

	bool Texture3DResource::write(ui32 mipLevel, const Box<ui32>& range, const void* data) {
		D3D11_BOX box;
		box.front = range.front;
		box.back = range.back;
		box.top = range.top;
		box.bottom = range.bottom;
		box.left = range.left;
		box.right = range.right;

		return _baseTexRes.write(_graphics.get<Graphics>(), mipLevel, box, data);
	}
}