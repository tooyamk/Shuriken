#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseBuffer::BaseBuffer(UINT bufferType) : BaseResource(bufferType),
		mapUsage(Usage::NONE) {
	}

	BaseBuffer::~BaseBuffer() {
	}

	bool BaseBuffer::create(Graphics* graphics, ui32 size, Usage resUsage, const void* data, ui32 dataSize) {
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
			hr = graphics->getDevice()->CreateBuffer(&desc, &res, (ID3D11Buffer**)&handle);
		} else {
			hr = graphics->getDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&handle);
		}
		if (FAILED(hr)) {
			releaseBuffer(graphics);
			return false;
		}

		return true;
	}

	Usage BaseBuffer::map(Graphics* graphics, Usage expectMapUsage) {
		return BaseResource::map(graphics, expectMapUsage, mapUsage, 0, mappedRes);
	}

	void BaseBuffer::unmap(Graphics* graphics) {
		BaseResource::unmap(graphics, mapUsage, 0);
	}

	i32 BaseBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		if ((mapUsage & Usage::CPU_READ) == Usage::CPU_READ) {
			if (dstLen == 0 || readLen == 0 || offset >= size) return 0;
			if (dst) {
				if (readLen < 0) readLen = size - offset;
				if ((ui32)readLen > dstLen) readLen = dstLen;
				memcpy(dst, (i8*)mappedRes.pData + offset, readLen);
				return readLen;
			}
		}
		return -1;
	}

	i32 BaseBuffer::write(Graphics* graphics, ui32 offset, const void* data, ui32 length) {
		if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)mappedRes.pData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	i32 BaseBuffer::update(Graphics* graphics, ui32 offset, const void* data, ui32 length) {
		if ((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				if (length == size) {
					graphics->getContext()->UpdateSubresource(handle, 0, nullptr, data, 0, 0);
				} else {
					D3D11_BOX box;
					box.back = 1;
					box.front = 0;
					box.top = 0;
					box.bottom = 1;
					box.left = offset;
					box.right = offset + length;
					graphics->getContext()->UpdateSubresource(handle, 0, &box, data, 0, 0);
				}
				return length;
			}
			return 0;
		}
		return -1;
	}

	void BaseBuffer::releaseBuffer(Graphics* graphics) {
		unmap(graphics);
		releaseRes();
	}
}