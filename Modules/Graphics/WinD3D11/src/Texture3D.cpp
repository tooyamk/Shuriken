#include "Texture3D.h"
#include "Graphics.h"
#include "math/Box.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture3D::Texture3D(Graphics& graphics) : ITexture3D(graphics),
		_res(graphics),
		_view(graphics) {
		_res.ref();
		_view.ref();
		_view.create(&_res, 0, -1);
	}

	Texture3D::~Texture3D() {
	}

	const void* Texture3D::getNative() const {
		return _view.getNative();
	}

	ui32 Texture3D::getMipLevels() const {
		return _res.getMipLevels();
	}

	bool Texture3D::create(const Size3<ui32>& size, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		return _res.create(size, format, mipLevels, resUsage, data);
	}

	Usage Texture3D::map(ui32 mipLevel, Usage expectMapUsage) {
		return _res.map(mipLevel, expectMapUsage);
	}

	void Texture3D::unmap(ui32 mipLevel) {
		_res.unmap(mipLevel);
	}

	i32 Texture3D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _res.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture3D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _res.write(mipLevel, offset, data, length);
	}

	bool Texture3D::write(ui32 mipLevel, const Box<ui32>& range, const void* data) {
		return _res.write(mipLevel, range, data);
	}
}