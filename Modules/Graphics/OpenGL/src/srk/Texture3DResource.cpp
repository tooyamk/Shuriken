#include "Texture3DResource.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
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

	TextureFormat Texture3DResource::getFormat() const {
		return _baseTex.format;
	}

	const Vec3uz& Texture3DResource::getDimensions() const {
		return _baseTex.dim;
	}

	bool Texture3DResource::create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data) {
		return _baseTex.create(*_graphics.get<Graphics>(), dim, arraySize, mipLevels, 1, format, requiredUsage, preferredUsage, data);
	}

	Usage Texture3DResource::getUsage() const {
		return _baseTex.resUsage;
	}

	Usage Texture3DResource::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
	}

	void Texture3DResource::unmap(size_t arraySlice, size_t mipSlice) {
		_baseTex.unmap(arraySlice, mipSlice);
	}

	size_t Texture3DResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	size_t Texture3DResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		return _baseTex.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture3DResource::destroy() {
		_baseTex.releaseTex();
	}

	bool Texture3DResource::update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) {
		return _baseTex.update(arraySlice, mipSlice, range, data);
	}

	bool Texture3DResource::copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		return _baseTex.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}

	bool Texture3DResource::copyFrom(size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		return _baseTex.copyFrom(*_graphics.get<Graphics>(), arraySlice, mipSlice, range, pixelBuffer);
	}
}