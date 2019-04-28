#include "Texture2D.h"
#include "Graphics.h"
#include "math/Rect.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture2D::Texture2D(Graphics& graphics) : ITexture2D(graphics),
		_res(graphics),
		_view(graphics) {
		_res.ref();
		_view.ref();
		_view.create(&_res, 0, -1);
	}

	Texture2D::~Texture2D() {
	}

	const void* Texture2D::getNative() const {
		return _view.getNative();
	}

	ui32 Texture2D::getMipLevels() const {
		return _res.getMipLevels();
	}

	bool Texture2D::create(const Size2<ui32>& size, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		return _res.create(size, format, mipLevels, resUsage, data);
	}

	Usage Texture2D::map(ui32 mipLevel, Usage expectMapUsage) {
		return _res.map(mipLevel, expectMapUsage);
	}

	void Texture2D::unmap(ui32 mipLevel) {
		_res.unmap(mipLevel);
	}

	i32 Texture2D::read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _res.read(mipLevel, offset, dst, dstLen, readLen);
	}

	i32 Texture2D::write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) {
		return _res.write(mipLevel, offset, data, length);
	}

	bool Texture2D::write(ui32 mipLevel, const Rect<ui32>& range, const void* data) {
		return _res.write(mipLevel, range, data);
	}
}