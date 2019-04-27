#include "Texture2D.h"
#include "math/Rect.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture2D::Texture2D(Graphics& graphics) : ITexture2D(graphics),
		_baseTex(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture2D::~Texture2D() {
		_baseTex.releaseTex((Graphics*)_graphics);
	}

	TextureType Texture2D::getType() const {
		return TextureType::TEX2D;
	}

	const void* Texture2D::getNative() const {
		return &_baseTex;
	}

	bool Texture2D::allocate(ui32 width, ui32 height, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_baseTex.releaseTex((Graphics*)_graphics);
		return _baseTex.allocate((Graphics*)_graphics, TextureType::TEX2D, width, height, 1, -1, format, mipLevels, resUsage, data);
	}

	Usage Texture2D::map(ui32 mipLevel, Usage expectMapUsage) {
		return _baseTex.map((Graphics*)_graphics, mipLevel, expectMapUsage);
	}

	void Texture2D::unmap(ui32 mipLevel) {
		return _baseTex.unmap((Graphics*)_graphics, mipLevel);
	}

	i32 Texture2D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTex.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture2D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _baseTex.write(mipLevel, offset, data, length);
	}

	bool Texture2D::write(ui32 mipLevel, const Rect<ui32>& range, const void* data) {
		D3D11_BOX box;
		box.front = 0;
		box.back = 1;
		box.top = range.top;
		box.bottom = range.bottom;
		box.left = range.left;
		box.right = range.right;

		return _baseTex.write((Graphics*)_graphics, mipLevel, box, data);
	}
}