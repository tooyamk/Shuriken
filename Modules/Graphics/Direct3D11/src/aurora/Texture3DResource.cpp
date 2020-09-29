#include "Texture3DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::d3d11 {
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

	uint16_t Texture3DResource::getPerPixelByteSize() const {
		return _baseTexRes.perPixelSize;
	}

	const Vec3ui32& Texture3DResource::getSize() const {
		return (Vec3ui32&)_baseTexRes.texSize;
	}

	bool Texture3DResource::create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data) {
		return _baseTexRes.create(*_graphics.get<Graphics>(), TextureType::TEX3D, size, arraySize, mipLevels, 1, format, resUsage, data);
	}

	Usage Texture3DResource::getUsage() const {
		return _baseTexRes.resUsage;
	}

	Usage Texture3DResource::map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
		return _baseTexRes.map(*_graphics.get<Graphics>(), arraySlice, mipSlice, expectMapUsage);
	}

	void Texture3DResource::unmap(uint32_t arraySlice, uint32_t mipSlice) {
		_baseTexRes.unmap(*_graphics.get<Graphics>(), arraySlice, mipSlice);
	}

	uint32_t Texture3DResource::read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseTexRes.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	uint32_t Texture3DResource::write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		return _baseTexRes.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture3DResource::destroy() {
		_baseTexRes.releaseTex(*_graphics.get<Graphics>());
	}

	bool Texture3DResource::update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) {
		D3D11_BOX box;
		box.left = range.pos[0];
		box.right = range.pos[0] + range.size[0];
		box.top = range.pos[1];
		box.bottom = range.pos[1] + range.size[1];
		box.front = range.pos[2];
		box.back = range.pos[2] + range.size[2];
		
		return _baseTexRes.update(*_graphics.get<Graphics>(), arraySlice, mipSlice, box, data);
	}

	bool Texture3DResource::copyFrom(const Vec3ui32& dstPos, uint32_t dstArraySlice, uint32_t dstMipSlice, const ITextureResource* src, uint32_t srcArraySlice, uint32_t srcMipSlice, const Box3ui32& srcRange) {
		return _baseTexRes.copyFrom(*_graphics.get<Graphics>(), dstPos, dstArraySlice, dstMipSlice, src, srcArraySlice, srcMipSlice, srcRange);
	}

	bool Texture3DResource::copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) {
		return false;
	}

	bool Texture3DResource::copyTo(uint32_t mipSlice, const IPixelBuffer* pixelBuffer) {
		return false;
	}
}