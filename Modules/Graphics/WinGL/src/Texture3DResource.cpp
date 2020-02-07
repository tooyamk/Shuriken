#include "Texture3DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	Texture3DResource::Texture3DResource(Graphics& graphics) : ITexture3DResource(graphics),
		_baseTex(TextureType::TEX3D) {
	}

	Texture3DResource::~Texture3DResource() {
	}

	TextureType Texture3DResource::getType() const {
		return _baseTex.texType;
	}

	bool Texture3DResource::isCreated() const {
		return _baseTex.handle;
	}

	const void* Texture3DResource::getNative() const {
		return &_baseTex;
	}

	SampleCount Texture3DResource::getSampleCount() const {
		return _baseTex.sampleCount;
	}

	uint16_t Texture3DResource::getPerPixelByteSize() const {
		return _baseTex.perPixelSize;
	}

	const Vec3ui32& Texture3DResource::getSize() const {
		return _baseTex.texSize;
	}

	bool Texture3DResource::create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void* const* data) {
		return _baseTex.create(*_graphics.get<Graphics>(), Vec3ui32(size[0], size[1], 1), arraySize, mipLevels, 1, format, resUsage, data);
	}

	Usage Texture3DResource::getUsage() const {
		return _baseTex.resUsage;
	}

	Usage Texture3DResource::map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
		return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
	}

	void Texture3DResource::unmap(uint32_t arraySlice, uint32_t mipSlice) {
		_baseTex.unmap(arraySlice, mipSlice);
	}

	uint32_t Texture3DResource::read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	uint32_t Texture3DResource::write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		return _baseTex.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture3DResource::destroy() {
		_baseTex.releaseTex();
	}

	bool Texture3DResource::update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) {
		return _baseTex.update(arraySlice, mipSlice, range, data);
	}

	bool Texture3DResource::copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) {
		return _baseTex.copyFrom(*_graphics.get<Graphics>(), arraySlice, mipSlice, range, pixelBuffer);
	}
}