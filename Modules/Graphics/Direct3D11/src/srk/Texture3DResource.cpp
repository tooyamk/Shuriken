#include "Texture3DResource.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	Texture3DResource::Texture3DResource(Graphics& graphics) : ITexture3DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE) {
	}

	Texture3DResource::~Texture3DResource() {
		destroy();
	}

	TextureType Texture3DResource::getType() const {
		return TextureType::TEX3D;
	}

	bool Texture3DResource::isCreated() const {
		return _baseTexRes.handle;
	}

	const void* Texture3DResource::getNative() const {
		return &_baseTexRes;
	}

	SampleCount Texture3DResource::getSampleCount() const {
		return _baseTexRes.sampleCount;
	}

	TextureFormat Texture3DResource::getFormat() const {
		return _baseTexRes.format;
	}

	const Vec3uz& Texture3DResource::getDimensions() const {
		return _baseTexRes.dim;
	}

	bool Texture3DResource::create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data) {
		return _baseTexRes.create(*_graphics.get<Graphics>(), TextureType::TEX3D, dim, arraySize, mipLevels, 1, format, requiredUsage, preferredUsage, data);
	}

	Usage Texture3DResource::getUsage() const {
		return _baseTexRes.resUsage;
	}

	Usage Texture3DResource::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		return _baseTexRes.map(*_graphics.get<Graphics>(), arraySlice, mipSlice, expectMapUsage);
	}

	void Texture3DResource::unmap(size_t arraySlice, size_t mipSlice) {
		_baseTexRes.unmap(*_graphics.get<Graphics>(), arraySlice, mipSlice);
	}

	size_t Texture3DResource::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		return _baseTexRes.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	size_t Texture3DResource::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		return _baseTexRes.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture3DResource::destroy() {
		_baseTexRes.releaseTex(*_graphics.get<Graphics>());
	}

	bool Texture3DResource::update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) {
		D3D11_BOX box;
		box.left = range.pos[0];
		box.right = range.pos[0] + range.size[0];
		box.top = range.pos[1];
		box.bottom = range.pos[1] + range.size[1];
		box.front = range.pos[2];
		box.back = range.pos[2] + range.size[2];
		
		return _baseTexRes.update(*_graphics.get<Graphics>(), arraySlice, mipSlice, box, data);
	}

	bool Texture3DResource::copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		return _baseTexRes.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}

	bool Texture3DResource::copyFrom(size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		return false;
	}
}