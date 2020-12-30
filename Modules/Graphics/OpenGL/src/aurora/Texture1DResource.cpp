#include "Texture1DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::gl {
	Texture1DResource::Texture1DResource(Graphics& graphics) : ITexture1DResource(graphics),
		_baseTex(TextureType::TEX2D) {
	}

	Texture1DResource::~Texture1DResource() {
	}

	TextureType Texture1DResource::getType() const {
		return _baseTex.texType;
	}

	bool Texture1DResource::isCreated() const {
		return _baseTex.handle;
	}

	const void* Texture1DResource::getNative() const {
		return &_baseTex;
	}

	SampleCount Texture1DResource::getSampleCount() const {
		return _baseTex.sampleCount;
	}

	TextureFormat Texture1DResource::getFormat() const {
		return _baseTex.format;
	}

	uint16_t Texture1DResource::getPerPixelByteSize() const {
		return _baseTex.perPixelSize;
	}

	const Vec3ui32& Texture1DResource::getSize() const {
		return _baseTex.texSize;
	}

	bool Texture1DResource::create(uint32_t width, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void* const* data) {
		return _baseTex.create(*_graphics.get<Graphics>(), Vec3ui32(width, 1, 1), arraySize, mipLevels, 1, format, resUsage, data);
	}

	Usage Texture1DResource::getUsage() const {
		return _baseTex.resUsage;
	}

	Usage Texture1DResource::map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
		return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
	}

	void Texture1DResource::unmap(uint32_t arraySlice, uint32_t mipSlice) {
		_baseTex.unmap(arraySlice, mipSlice);
	}

	uint32_t Texture1DResource::read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	uint32_t Texture1DResource::write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		return _baseTex.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture1DResource::destroy() {
		_baseTex.releaseTex();
	}

	bool Texture1DResource::update(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const void* data) {
		Box3ui32 box;
		((Vec1ui32&)box.pos).set(range.pos.cast<1>());
		((Vec1ui32&)box.size).set(range.size.cast<1>());

		return _baseTex.update(arraySlice, mipSlice, box, data);
	}

	bool Texture1DResource::copyFrom(const Vec3ui32& dstPos, uint32_t dstArraySlice, uint32_t dstMipSlice, const ITextureResource* src, uint32_t srcArraySlice, uint32_t srcMipSlice, const Box3ui32& srcRange) {
		return _baseTex.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}

	bool Texture1DResource::copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) {
		return _baseTex.copyFrom(*_graphics.get<Graphics>(), arraySlice, mipSlice, range, pixelBuffer);
	}

	bool Texture1DResource::copyTo(uint32_t mipSlice, const IPixelBuffer* pixelBuffer) {
		return _baseTex.copyTo(*_graphics.get<Graphics>(), mipSlice, pixelBuffer);
	}
}