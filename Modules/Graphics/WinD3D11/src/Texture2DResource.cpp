#include "Texture2DResource.h"
#include "Graphics.h"
#include "math/Rect.h"
#include "math/Size2.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture2DResource::Texture2DResource(Graphics& graphics) : ITexture2DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE),
		_view(graphics, true) {
	}

	Texture2DResource::~Texture2DResource() {
		_baseTexRes.releaseTex(_graphics.get<Graphics>());
	}

	TextureType Texture2DResource::getType() const {
		return TextureType::TEX2D;
	}

	const void* Texture2DResource::getNativeView() const {
		return _view.getNativeView();
	}

	const void* Texture2DResource::getNativeResource() const {
		return &_baseTexRes;
	}

	ui32 Texture2DResource::getArraySize() const {
		return _baseTexRes.arraySize;
	}

	ui32 Texture2DResource::getMipLevels() const {
		return _baseTexRes.mipLevels;
	}

	bool Texture2DResource::create(const Size2<ui32>& size, ui32 arraySize, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data) {
		auto rst = _baseTexRes.create(_graphics.get<Graphics>(), TextureType::TEX2D, Size3<ui32>(size.width, size.height, 1), arraySize, format, mipLevels, resUsage, data);
		_view.create(this, 0, -1, 0, _baseTexRes.arraySize);
		return rst;
	}

	Usage Texture2DResource::map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) {
		return _baseTexRes.map(_graphics.get<Graphics>(), arraySlice, mipSlice, expectMapUsage);
	}

	void Texture2DResource::unmap(ui32 arraySlice, ui32 mipSlice) {
		_baseTexRes.unmap(_graphics.get<Graphics>(), arraySlice, mipSlice);
	}

	i32 Texture2DResource::read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTexRes.read(arraySlice, mipSlice, offset, dst, dstLen, readLen);
	}

	i32 Texture2DResource::write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) {
		return _baseTexRes.write(arraySlice, mipSlice, offset, data, length);
	}

	bool Texture2DResource::write(ui32 arraySlice, ui32 mipSlice, const Rect<ui32>& range, const void* data) {
		D3D11_BOX box;
		box.front = 0;
		box.back = 1;
		box.top = range.top;
		box.bottom = range.top + range.size.height;
		box.left = range.left;
		box.right = range.left + range.size.width;

		return _baseTexRes.write(_graphics.get<Graphics>(), arraySlice, mipSlice, box, data);
	}
}