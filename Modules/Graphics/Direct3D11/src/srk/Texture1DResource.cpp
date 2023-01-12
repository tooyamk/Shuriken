#include "Texture1DResource.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	Texture1DResource::Texture1DResource(Graphics& graphics) : ITexture1DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture1DResource::~Texture1DResource() {
		destroy();
	}

	TextureType Texture1DResource::getType() const {
		return TextureType::TEX1D;
	}

	bool Texture1DResource::isCreated() const {
		return _baseTexRes.handle;
	}

	const void* Texture1DResource::getNative() const {
		return &_baseTexRes;
	}

	SampleCount Texture1DResource::getSampleCount() const {
		return _baseTexRes.sampleCount;
	}

	TextureFormat Texture1DResource::getFormat() const {
		return _baseTexRes.format;
	}

	const Vec3uz& Texture1DResource::getDimensions() const {
		return _baseTexRes.dim;
	}

	bool Texture1DResource::create(size_t width, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data) {
		return _baseTexRes.create(*_graphics.get<Graphics>(), TextureType::TEX1D, Vec3uz(width, 1, 1), arraySize, mipLevels, 1, format, requiredUsage, preferredUsage, data);
	}

	Usage Texture1DResource::getUsage() const {
		return _baseTexRes.resUsage;
	}

	Usage Texture1DResource::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		return _baseTexRes.map(*_graphics.get<Graphics>(), arraySlice, mipSlice, expectMapUsage);
	}

	void Texture1DResource::unmap(size_t arraySlice, size_t mipSlice) {
		_baseTexRes.unmap(*_graphics.get<Graphics>(), arraySlice, mipSlice);
	}

	size_t Texture1DResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		return _baseTexRes.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	size_t Texture1DResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		return _baseTexRes.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture1DResource::destroy() {
		_baseTexRes.releaseTex(*_graphics.get<Graphics>());
	}

	bool Texture1DResource::update(size_t arraySlice, size_t mipSlice, const Box1uz& range, const void* data) {
		D3D11_BOX box;
		box.left = range.pos[0];
		box.right = range.pos[0] + range.size[0];
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;

		return _baseTexRes.update(*_graphics.get<Graphics>(), arraySlice, mipSlice, box, data);
	}

	bool Texture1DResource::copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		return _baseTexRes.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}

	bool Texture1DResource::copyFrom(size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		return false;
	}
}