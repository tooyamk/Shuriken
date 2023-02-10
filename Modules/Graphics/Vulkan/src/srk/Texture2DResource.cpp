#include "Texture2DResource.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	Texture2DResource::Texture2DResource(Graphics& graphics) : ITexture2DResource(graphics),
		_baseTex(TextureType::TEX2D) {
	}

	Texture2DResource::~Texture2DResource() {
	}

	TextureType Texture2DResource::getType() const {
		return _baseTex.getTexType();
	}

	bool Texture2DResource::isCreated() const {
		return _baseTex.getVkImage();
	}

	const void* Texture2DResource::getNative() const {
		return &_baseTex;
	}

	SampleCount Texture2DResource::getSampleCount() const {
		return _baseTex.getSampleCount();
	}

	TextureFormat Texture2DResource::getFormat() const {
		return _baseTex.getFormat();
	}

	const Vec3uz& Texture2DResource::getDimensions() const {
		return _baseTex.getDimensions();
	}

	bool Texture2DResource::create(const Vec2uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data) {
		return _baseTex.create(*_graphics.get<Graphics>(), Vec3uz(dim[0], dim[1], 1), arraySize, mipLevels, sampleCount, format, requiredUsage, preferredUsage, data);
	}

	Usage Texture2DResource::getUsage() const {
		return _baseTex.getUsage();
	}

	Usage Texture2DResource::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		//return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
		return Usage::NONE;
	}

	void Texture2DResource::unmap(size_t arraySlice, size_t mipSlice) {
		//_baseTex.unmap(arraySlice, mipSlice);
	}

	size_t Texture2DResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		//return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen);
		return -1;
	}

	size_t Texture2DResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		//return _baseTex.write(arraySlice, mipSlice, offset, data, length);
		return -1;
	}

	void Texture2DResource::destroy() {
		_baseTex.destroy();
	}

	bool Texture2DResource::update(size_t arraySlice, size_t mipSlice, const Box2uz& range, const void* data) {
		return _baseTex.update(arraySlice, mipSlice, Box3uz(Vec3uz(range.pos[0], range.pos[1], 0), Vec3uz(range.size[0], range.size[1], 1)), data);
	}

	bool Texture2DResource::copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		//return _baseTex.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
		return false;
	}

	bool Texture2DResource::copyFrom(size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		//return _baseTex.copyFrom(*_graphics.get<Graphics>(), arraySlice, mipSlice, range, pixelBuffer);
		return false;
	}
}