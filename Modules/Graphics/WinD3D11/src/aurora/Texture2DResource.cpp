#include "Texture2DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
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

	uint16_t Texture2DResource::getPerPixelByteSize() const {
		return _baseTexRes.perPixelSize;
	}

	const Vec2ui32& Texture2DResource::getSize() const {
		return (Vec2ui32&)_baseTexRes.texSize;
	}

	bool Texture2DResource::create(const Vec2ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage resUsage, const void*const* data) {
		return _baseTexRes.create(*_graphics.get<Graphics>(), TextureType::TEX2D, Vec3ui32(size[0], size[1], 1), arraySize, mipLevels, sampleCount, format, resUsage, data);
	}

	Usage Texture2DResource::getUsage() const {
		return _baseTexRes.resUsage;
	}

	Usage Texture2DResource::map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
		return _baseTexRes.map(*_graphics.get<Graphics>(), arraySlice, mipSlice, expectMapUsage);
	}

	void Texture2DResource::unmap(uint32_t arraySlice, uint32_t mipSlice) {
		_baseTexRes.unmap(*_graphics.get<Graphics>(), arraySlice, mipSlice);
	}

	uint32_t Texture2DResource::read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseTexRes.read(arraySlice, mipSlice, offset, dst, dstLen);
	}

	uint32_t Texture2DResource::write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		return _baseTexRes.write(arraySlice, mipSlice, offset, data, length);
	}

	void Texture2DResource::destroy() {
		_baseTexRes.releaseTex(*_graphics.get<Graphics>());
	}

	bool Texture2DResource::update(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const void* data) {
		D3D11_BOX box;
		box.left = range.pos[0];
		box.right = range.pos[0] + range.size[0];
		box.top = range.pos[1];
		box.bottom = range.pos[1] + range.size[1];
		box.front = 0;
		box.back = 1;

		return _baseTexRes.update(*_graphics.get<Graphics>(), arraySlice, mipSlice, box, data);
	}

	bool Texture2DResource::copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const IPixelBuffer* pixelBuffer) {
		return false;
	}
}