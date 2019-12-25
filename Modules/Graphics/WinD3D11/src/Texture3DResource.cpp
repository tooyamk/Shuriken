#include "Texture3DResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	Texture3DResource::Texture3DResource(Graphics& graphics) : ITexture3DResource(graphics),
		_baseTexRes(D3D11_BIND_SHADER_RESOURCE),
		_view(graphics, true) {
	}

	Texture3DResource::~Texture3DResource() {
		_baseTexRes.releaseTex(*_graphics.get<Graphics>());
	}

	TextureType Texture3DResource::getType() const {
		return TextureType::TEX3D;
	}

	const void* Texture3DResource::getNativeView() const {
		return _view.getNativeView();
	}

	const void* Texture3DResource::getNativeResource() const {
		return &_baseTexRes;
	}

	uint16_t Texture3DResource::getPerPixelByteSize() const {
		return _baseTexRes.perPixelSize;
	}

	uint32_t Texture3DResource::getArraySize() const {
		return _baseTexRes.arraySize;
	}

	uint32_t Texture3DResource::getMipLevels() const {
		return _baseTexRes.mipLevels;
	}

	bool Texture3DResource::create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data) {
		auto rst = _baseTexRes.create(*_graphics.get<Graphics>(), TextureType::TEX3D, size, arraySize, mipLevels, format, resUsage, data);
		_view.create(this, 0, -1, 0, _baseTexRes.arraySize);
		return rst;
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

	bool Texture3DResource::copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) {
		return false;
	}
}