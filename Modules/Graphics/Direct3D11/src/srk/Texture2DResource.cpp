#include "Texture2DResource.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	Texture2DResource::Texture2DResource(Graphics& graphics) : ITexture2DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture2DResource::~Texture2DResource() {
		destroy();
	}

	TextureType Texture2DResource::getType() const {
		return TextureType::TEX2D;
	}

	bool Texture2DResource::isCreated() const {
		return _baseTexRes.handle;
	}

	const void* Texture2DResource::getNative() const {
		return &_baseTexRes;
	}

	SampleCount Texture2DResource::getSampleCount() const {
		return _baseTexRes.sampleCount;
	}

	TextureFormat Texture2DResource::getFormat() const {
		return _baseTexRes.format;
	}

	const Vec3uz& Texture2DResource::getDimensions() const {
		return _baseTexRes.dim;
	}

	bool Texture2DResource::create(const Vec2uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data) {
		return _baseTexRes.create(*_graphics.get<Graphics>(), TextureType::TEX2D, Vec3uz(dim[0], dim[1], 1), arraySize, mipLevels, sampleCount, format, requiredUsage, preferredUsage, data);
	}

	Usage Texture2DResource::getUsage() const {
		return _baseTexRes.resUsage;
	}

	Usage Texture2DResource::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		return _baseTexRes.map(*_graphics.get<Graphics>(), arraySlice, mipSlice, expectMapUsage);
	}

	void Texture2DResource::unmap(size_t arraySlice, size_t mipSlice) {
		_baseTexRes.unmap(*_graphics.get<Graphics>(), arraySlice, mipSlice);
	}

	size_t Texture2DResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		return _baseTexRes.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	size_t Texture2DResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		return _baseTexRes.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture2DResource::destroy() {
		_baseTexRes.releaseTex(*_graphics.get<Graphics>());
	}

	bool Texture2DResource::update(size_t arraySlice, size_t mipSlice, const Box2uz& range, const void* data) {
		D3D11_BOX box;
		box.left = range.pos[0];
		box.right = range.pos[0] + range.size[0];
		box.top = range.pos[1];
		box.bottom = range.pos[1] + range.size[1];
		box.front = 0;
		box.back = 1;

		return _baseTexRes.update(*_graphics.get<Graphics>(), arraySlice, mipSlice, box, data);
	}

	bool Texture2DResource::copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		return _baseTexRes.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}

	bool Texture2DResource::copyFrom(size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		return false;
	}
}