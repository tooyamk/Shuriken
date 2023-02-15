#include "Texture1DResource.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	Texture1DResource::Texture1DResource(Graphics& graphics) : ITexture1DResource(graphics),
		_baseTex(TextureType::TEX2D) {
	}

	Texture1DResource::~Texture1DResource() {
	}

	TextureType Texture1DResource::getType() const {
		return _baseTex.getTexType();
	}

	bool Texture1DResource::isCreated() const {
		return _baseTex.getVkImage();
	}

	const void* Texture1DResource::getNative() const {
		return &_baseTex;
	}

	SampleCount Texture1DResource::getSampleCount() const {
		return _baseTex.getSampleCount();
	}

	TextureFormat Texture1DResource::getFormat() const {
		return _baseTex.getFormat();
	}

	const Vec3uz& Texture1DResource::getDimensions() const {
		return _baseTex.getDimensions();
	}

	bool Texture1DResource::create(size_t width, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data) {
		return _baseTex.create(*_graphics.get<Graphics>(), Vec3uz(width, 1, 1), arraySize, mipLevels, 1, format, requiredUsage, preferredUsage, data);
	}

	Usage Texture1DResource::getUsage() const {
		return _baseTex.getUsage();
	}

	Usage Texture1DResource::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		return _baseTex.map(arraySlice, mipSlice, expectMapUsage);
	}

	void Texture1DResource::unmap(size_t arraySlice, size_t mipSlice) {
		_baseTex.unmap(arraySlice, mipSlice);
	}

	size_t Texture1DResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		return _baseTex.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	size_t Texture1DResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		return _baseTex.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture1DResource::destroy() {
		_baseTex.destroy();
	}

	bool Texture1DResource::update(size_t arraySlice, size_t mipSlice, const Box1uz& range, const void* data) {
		return _baseTex.update(arraySlice, mipSlice, Box3uz(Vec3uz(range.pos[0], 0, 0), Vec3uz(range.size[0], 1, 1)), data);
	}

	bool Texture1DResource::copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		return _baseTex.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}
}