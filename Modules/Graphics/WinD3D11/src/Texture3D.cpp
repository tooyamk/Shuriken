#include "Texture3D.h"
#include "math/Box.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture3D::Texture3D(Graphics& graphics) : ITexture3D(graphics),
		_baseTex(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture3D::~Texture3D() {
		_baseTex.releaseTex((Graphics*)_graphics);
	}

	TextureType Texture3D::getType() const {
		return TextureType::TEX3D;
	}

	const void* Texture3D::getNative() const {
		return &_baseTex;
	}

	bool Texture3D::allocate(ui32 width, ui32 height, ui32 depth, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		_baseTex.releaseTex((Graphics*)_graphics);
		return _baseTex.allocate((Graphics*)_graphics, TextureType::TEX3D, width, height, depth, -1, format, mipLevels, resUsage, data);
	}

	Usage Texture3D::map(ui32 mipLevel, Usage expectMapUsage) {
		return _baseTex.map((Graphics*)_graphics, mipLevel, expectMapUsage);
	}

	void Texture3D::unmap(ui32 mipLevel) {
		return _baseTex.unmap((Graphics*)_graphics, mipLevel);
	}

	i32 Texture3D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTex.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture3D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _baseTex.write(mipLevel, offset, data, length);
	}

	bool Texture3D::write(ui32 mipLevel, const Box<ui32>& range, const void* data) {
		D3D11_BOX box;
		box.front = range.front;
		box.back = range.back;
		box.top = range.top;
		box.bottom = range.bottom;
		box.left = range.left;
		box.right = range.right;

		return _baseTex.write((Graphics*)_graphics, mipLevel, box, data);
	}
}