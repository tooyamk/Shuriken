#include "Texture1D.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture1D::Texture1D(Graphics& graphics) : ITexture1D(graphics),
		_res(graphics),
		_view(graphics) {
		_res.ref();
		_view.ref();
		_view.create(&_res, 0, -1);
	}

	Texture1D::~Texture1D() {
	}

	const void* Texture1D::getNative() const {
		return _view.getNative();
	}

	ui32 Texture1D::getMipLevels() const {
		return _res.getMipLevels();
	}

	bool Texture1D::create(ui32 width, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		return _res.create(width, format, mipLevels, resUsage, data);
	}

	Usage Texture1D::map(ui32 mipLevel, Usage expectMapUsage) {
		return _res.map(mipLevel, expectMapUsage);
	}

	void Texture1D::unmap(ui32 mipLevel) {
		_res.unmap(mipLevel);
	}

	i32 Texture1D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _res.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture1D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _res.write(mipLevel, offset, data, length);
	}

	bool Texture1D::write(ui32 mipLevel, ui32 left, ui32 right, const void* data) {
		return _res.write(mipLevel, left, right, data);
	}
}