#include "Texture1D.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture1D::Texture1D(Graphics& graphics) : ITexture1D(graphics),
		_baseTex(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture1D::~Texture1D() {
		_baseTex.releaseTex((Graphics*)_graphics);
	}

	TextureType Texture1D::getType() const {
		return TextureType::TEX1D;
	}

	void* Texture1D::getNative() const {
		return (void*)&_baseTex;
	}

	bool Texture1D::allocate(ui32 width, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_baseTex.releaseTex((Graphics*)_graphics);
		return _baseTex.allocate((Graphics*)_graphics, TextureType::TEX1D, width, 1, 1, format, mipLevels, resUsage, data);
	}

	Usage Texture1D::map(ui32 mipLevel, Usage expectMapUsage) {
		return _baseTex.map((Graphics*)_graphics, mipLevel, expectMapUsage);
	}

	void Texture1D::unmap(ui32 mipLevel) {
		return _baseTex.unmap((Graphics*)_graphics, mipLevel);
	}

	i32 Texture1D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTex.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture1D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _baseTex.write(mipLevel, offset, data, length);
	}

	bool Texture1D::write(ui32 mipLevel, ui32 left, ui32 right, const void* data) {
		D3D11_BOX box;
		box.front = 0;
		box.back = 1;
		box.top = 0;
		box.bottom = 1;
		box.left = left;
		box.right = right;

		return _baseTex.write((Graphics*)_graphics, mipLevel, box, data);
	}
}