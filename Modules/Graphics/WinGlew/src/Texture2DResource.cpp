#include "Texture2DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	Texture2DResource::Texture2DResource(Graphics& graphics) : ITexture2DResource(graphics),
		_baseTex(TextureType::TEX2D) {
	}

	Texture2DResource::~Texture2DResource() {
	}

	TextureType Texture2DResource::getType() const {
		return TextureType::TEX2D;
	}

	const void* Texture2DResource::getNativeView() const {
		return nullptr;
	}

	const void* Texture2DResource::getNativeResource() const {
		return &_baseTex.handle;
	}

	ui32 Texture2DResource::getArraySize() const {
		return _baseTex.arraySize;
	}

	ui32 Texture2DResource::getMipLevels() const {
		return _baseTex.mipLevels;
	}

	bool Texture2DResource::create(const Vec2ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data) {
		return _baseTex.create(_graphics.get<Graphics>(), Vec3ui32({ size[0], size[1], 1 }), arraySize, mipLevels, format, resUsage, data);
	}

	Usage Texture2DResource::getUsage() const {
		return _baseTex.resUsage;
	}

	Usage Texture2DResource::map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) {
		return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
	}

	void Texture2DResource::unmap(ui32 arraySlice, ui32 mipSlice) {
		_baseTex.unmap(arraySlice, mipSlice);
	}

	i32 Texture2DResource::read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen, readLen);
	}

	i32 Texture2DResource::write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) {
		return _baseTex.write(arraySlice, mipSlice, offset, data, length);
	}

	bool Texture2DResource::update(ui32 arraySlice, ui32 mipSlice, const Box2ui32& range, const void* data) {
		Box3ui32 box;
		((Vec2ui32&)box.pos).set((ui32(&)[2])range.pos);
		((Vec2ui32&)box.size).set((ui32(&)[2])range.size);

		return _baseTex.update(arraySlice, mipSlice, box, data);
	}
}