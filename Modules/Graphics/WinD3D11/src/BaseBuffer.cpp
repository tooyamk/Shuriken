#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseBuffer::BaseBuffer(UINT bufferType) : BaseResource(bufferType),
		mapUsage(Usage::NONE),
		mappedRes() {
	}

	BaseBuffer::~BaseBuffer() {
	}

	bool BaseBuffer::create(Graphics& graphics, uint32_t size, Usage resUsage, const void* data, uint32_t dataSize) {
		releaseBuffer(graphics);

		this->size = size;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		createInit(resUsage, size, dataSize, 0, cpuUsage, d3dUsage);

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

	uint32_t BaseBuffer::read (uint32_t offset, void* dst, uint32_t dstLen) {
		if ((mapUsage & Usage::MAP_READ) == Usage::MAP_READ) {
			if (!dstLen || offset >= size) return 0;
			if (dst) {
				auto readLen = std::min<uint32_t>(size - offset, dstLen);
				memcpy(dst, (uint8_t*)mappedRes.pData + offset, readLen);
				return readLen;
			}
		}
		return -1;
	}

	uint32_t BaseBuffer::write(Graphics& graphics, uint32_t offset, const void* data, uint32_t length) {
		if ((mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (data && length && offset < size) {
				length = std::min<uint32_t>(length, size - offset);
				memcpy((uint8_t*)mappedRes.pData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	uint32_t BaseBuffer::update(Graphics& graphics, uint32_t offset, const void* data, uint32_t length) {
		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && length && offset < size) {
				length = std::min<uint32_t>(length, size - offset);
				if (length == size) {
					graphics.getContext()->UpdateSubresource(handle, 0, nullptr, data, 0, 0);
				} else {
					D3D11_BOX box;
					box.back = 1;
					box.front = 0;
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

	void BaseBuffer::releaseBuffer(Graphics& graphics) {
		unmap(graphics);
		releaseRes();
	}
}