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
		return &_baseTex.handle;
	}

	const void* Texture2DResource::getNativeResource() const {
		return &_baseTex;
	}

	uint16_t Texture2DResource::getPerPixelByteSize() const {
		return _baseTex.perPixelSize;
	}

	uint32_t Texture2DResource::getArraySize() const {
		return _baseTex.isArray ? _baseTex.arraySize : 0;
	}

	uint32_t Texture2DResource::getMipLevels() const {
		return _baseTex.mipLevels;
	}

	bool Texture2DResource::create(const Vec2ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data) {
		return _baseTex.create(*_graphics.get<Graphics>(), Vec3ui32(size[0], size[1], 1), arraySize, mipLevels, format, resUsage, data);
	}

	Usage Texture2DResource::getUsage() const {
		return _baseTex.resUsage;
	}

	Usage Texture2DResource::map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
		return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
	}

	void Texture2DResource::unmap(uint32_t arraySlice, uint32_t mipSlice) {
		_baseTex.unmap(arraySlice, mipSlice);
	}

	uint32_t Texture2DResource::read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	uint32_t Texture2DResource::write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		return _baseTex.write(arraySlice, mipSlice, offset, data, length);
	}

	bool Texture2DResource::update(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const void* data) {
		Box3ui32 box;
		((Vec2ui32&)box.pos).set(range.pos.slice<2>());
		((Vec2ui32&)box.size).set(range.size.slice<2>());

		return _baseTex.update(arraySlice, mipSlice, box, data);
	}

	bool Texture2DResource::copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const IPixelBuffer* pixelBuffer) {
		Box3ui32 box;
		((Vec2ui32&)box.pos).set(range.pos.slice<2>());
		((Vec2ui32&)box.size).set(range.size.slice<2>());

		return _baseTex.copyFrom(*_graphics.get<Graphics>(), arraySlice, mipSlice, box, pixelBuffer);
	}
}