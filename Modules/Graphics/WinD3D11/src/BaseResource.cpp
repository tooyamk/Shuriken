#include "BaseResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseResource::BaseResource(UINT bindType) :
		size(0),
		_bindType(bindType),
		_resUsage(0),
		_mapUsage(0),
		_mapData(nullptr),
		handle(nullptr) {
	}

	BaseResource::~BaseResource() {
	}

	void BaseResource::calcAllocateUsage(ui32 resUsage, const void* data, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
		cpuUsage = 0;
		if (resUsage & BufferUsage::CPU_READ) cpuUsage |= D3D11_CPU_ACCESS_READ;
		if (resUsage & BufferUsage::CPU_WRITE) cpuUsage |= D3D11_CPU_ACCESS_WRITE;

		if ((resUsage & BufferUsage::CPU_READ) || ((resUsage & BufferUsage::CPU_GPU_WRITE) == BufferUsage::CPU_GPU_WRITE)) {
			d3dUsage = D3D11_USAGE_STAGING;
			_resUsage = BufferUsage::CPU_READ_WRITE | BufferUsage::GPU_WRITE;
		} else if (resUsage & BufferUsage::CPU_WRITE) {
			d3dUsage = D3D11_USAGE_DYNAMIC;
			_resUsage = BufferUsage::CPU_WRITE;
		} else if (resUsage & BufferUsage::GPU_WRITE || !data) {
			d3dUsage = D3D11_USAGE_DEFAULT;
			_resUsage = BufferUsage::GPU_WRITE;
		} else {
			d3dUsage = D3D11_USAGE_IMMUTABLE;
		}
	}

	ui32 BaseResource::map(Graphics* graphics, ui32 mapUsage) {
		ui32 ret = 0;

		mapUsage &= BufferUsage::CPU_READ | BufferUsage::CPU_WRITE;
		if (handle && mapUsage) {
			unmap(graphics);

			UINT mapType = 0;
			if ((mapUsage & BufferUsage::CPU_READ) && (_resUsage & BufferUsage::CPU_READ)) {
				mapType |= D3D11_MAP_READ;
				ret |= BufferUsage::CPU_READ;
			} else {
				mapUsage &= ~BufferUsage::CPU_READ;
			}
			if (mapUsage & BufferUsage::CPU_WRITE) {
				if (_resUsage & BufferUsage::CPU_WRITE) {
					ret |= BufferUsage::CPU_WRITE;
					if (_bindType == D3D11_BIND_CONSTANT_BUFFER) {
						if (graphics->getFeatureOptions().MapNoOverwriteOnDynamicConstantBuffer) {
							mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
							ret |= BufferUsage::CPU_WRITE_NO_OVERWRITE;
						} else {
							mapType |= D3D11_MAP_WRITE_DISCARD;
							ret |= BufferUsage::CPU_WRITE_DISCARD;
						}
					} else if (_bindType & (D3D11_BIND_VERTEX_BUFFER & D3D11_BIND_INDEX_BUFFER)) {
						mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
						ret |= BufferUsage::CPU_WRITE_NO_OVERWRITE;
					} else {
						mapType |= D3D11_MAP_WRITE_DISCARD;
						ret |= BufferUsage::CPU_WRITE_DISCARD;
					}
				} else if (_resUsage & BufferUsage::GPU_WRITE && !ret) {
					ret |= BufferUsage::GPU_WRITE;
				}
			} else {
				mapUsage &= ~BufferUsage::CPU_WRITE;
			}

			if (mapType) {
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

				if (SUCCEEDED(graphics->getContext()->Map(handle, 0, (D3D11_MAP)mapType, 0, &mappedResource))) {
					_mapUsage = mapUsage;
					_mapData = mappedResource.pData;
				} else {
					ret = _resUsage & BufferUsage::GPU_WRITE ? BufferUsage::GPU_WRITE : 0;
				}
			}
		}

		return ret;
	}

	void BaseResource::unmap(Graphics* graphics) {
		if (_mapUsage) {
			graphics->getContext()->Unmap(handle, 0);
			_mapData = nullptr;
			_mapUsage = 0;
		}
	}

	i32 BaseResource::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		if (_mapUsage & BufferUsage::CPU_READ) {
			if (dstLen == 0 || readLen == 0 || offset >= size) return 0;
			if (dst) {
				if (readLen < 0) readLen = size - offset;
				if ((ui32)readLen > dstLen) readLen = dstLen;
				memcpy(dst, (i8*)_mapData + offset, readLen);
				return readLen;
			}
		}
		return -1;
	}

	i32 BaseResource::write(Graphics* graphics, ui32 offset, const void* data, ui32 length) {
		if (_mapUsage & BufferUsage::CPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)_mapData + offset, data, length);
				return length;
			}
			return 0;
		} else if (_resUsage & BufferUsage::GPU_WRITE) {
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

	void BaseResource::flush() {
		if (handle && (_resUsage & BufferUsage::GPU_WRITE)) {

		}
	}

	void BaseResource::releaseRes(Graphics* graphics) {
		if (handle) {
			unmap(graphics);

			handle->Release();
			handle = nullptr;
		}
		size = 0;
		_resUsage = 0;
	}

	void BaseResource::_waitServerSync() {
	}

	void BaseResource::_delSync() {
	}
}