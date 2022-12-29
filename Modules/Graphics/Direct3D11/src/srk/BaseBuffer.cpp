#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace srk::modules::graphics::d3d11 {
	BaseBuffer::BaseBuffer(D3D11_BIND_FLAG bufferType) : BaseResource(bufferType),
		mapUsage(Usage::NONE),
		mappedRes() {
	}

	BaseBuffer::~BaseBuffer() {
	}

	bool BaseBuffer::create(Graphics& graphics, size_t size, Usage resUsage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		releaseBuffer(graphics);

		this->size = size;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		if (!createInit<false>(graphics, resUsage & Usage::CREATE_ALL, size, dataSize, 0, cpuUsage, d3dUsage)) return false;

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = bindType;
		desc.ByteWidth = size;

		HRESULT hr;
		if (dataSize >= size) {
			D3D11_SUBRESOURCE_DATA res;
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
			hr = graphics.getDevice()->CreateBuffer(&desc, &res, (ID3D11Buffer**)&handle);
		} else {
			hr = graphics.getDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&handle);
		}
		if (FAILED(hr)) {
			releaseBuffer(graphics);
			return false;
		}

		return true;
	}

	Usage BaseBuffer::map(Graphics& graphics, Usage expectMapUsage) {
		return BaseResource::map(graphics, expectMapUsage, mapUsage, 0, mappedRes);
	}

	void BaseBuffer::unmap(Graphics& graphics) {
		BaseResource::unmap(graphics, mapUsage, 0);
	}

	size_t BaseBuffer::read(void* dst, size_t dstLen, size_t offset) {
		using namespace srk::enum_operators;

		if ((mapUsage & Usage::MAP_READ) == Usage::MAP_READ) {
			if (!dstLen || offset >= size) return 0;
			if (dst) {
				auto readLen = std::min<size_t>(size - offset, dstLen);
				memcpy(dst, (uint8_t*)mappedRes.pData + offset, readLen);
				return readLen;
			}
		}
		return -1;
	}

	size_t BaseBuffer::write(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (data && length && offset < size) {
				length = std::min<size_t>(length, size - offset);
				memcpy((uint8_t*)mappedRes.pData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	size_t BaseBuffer::update(Graphics& graphics, const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && length && offset < size) {
				length = std::min<size_t>(length, size - offset);
				if (resType == D3D11_BIND_CONSTANT_BUFFER || length == size) {
					graphics.getContext()->UpdateSubresource(handle, 0, nullptr, data, 0, 0);
				} else {
					D3D11_BOX box;
					box.front = 0;
					box.back = 1;
					box.top = 0;
					box.bottom = 1;
					box.left = offset;
					box.right = offset + length;
					graphics.getContext()->UpdateSubresource(handle, 0, &box, data, 0, 0);
				}
				return length;
			}
			return 0;
		}
		return -1;
	}

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange) {
		if (dstPos >= size || srcRange.pos[0] >= src.size) return 0;

		auto copySize = std::min(std::min(src.size - srcRange.pos[0], srcRange.size[0]), size - dstPos);

		D3D11_BOX box;
		box.front = 0;
		box.back = 1;
		box.top = 0;
		box.bottom = 1;
		box.left = srcRange.pos[0];
		box.right = srcRange.pos[0] + copySize;
		graphics.getContext()->CopySubresourceRegion(handle, 0, 0, 0, 0, src.handle, 0, &box);

		return copySize;
	}

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		using namespace srk::enum_operators;

		if (!handle || (resUsage & Usage::COPY_DST) != Usage::COPY_DST || !src || (src->getUsage() & Usage::COPY_SRC) != Usage::COPY_SRC || src->getGraphics() != graphics) return -1;

		auto srcNative = (const BaseBuffer*)src->getNative();
		if (!srcNative || !srcNative->handle) return -1;

		return copyFrom(graphics, dstPos, *srcNative, srcRange);
	}

	void BaseBuffer::releaseBuffer(Graphics& graphics) {
		unmap(graphics);
		releaseRes();
	}
}