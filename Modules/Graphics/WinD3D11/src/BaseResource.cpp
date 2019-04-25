#include "BaseResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseResource::BaseResource(UINT bindType) :
		size(0),
		_bindType(bindType),
		_resUsage(Usage::NONE),
		_mapUsage(Usage::NONE),
		_mapData(nullptr),
		handle(nullptr) {
	}

	BaseResource::~BaseResource() {
	}

	void BaseResource::calcAllocateUsage(Usage resUsage, ui32 resSize, ui32 dataSize, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
		cpuUsage = 0;
		
		bool resCpuRead = (resUsage & Usage::CPU_READ) == Usage::CPU_READ;
		bool resCpuWrite = (resUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE;
		if (resCpuRead) cpuUsage |= D3D11_CPU_ACCESS_READ;
		if (resCpuWrite) cpuUsage |= D3D11_CPU_ACCESS_WRITE;

		if (resCpuRead || ((resUsage & Usage::CPU_GPU_WRITE) == Usage::CPU_GPU_WRITE)) {
			d3dUsage = D3D11_USAGE_STAGING;
			_resUsage = Usage::CPU_READ_WRITE | Usage::GPU_WRITE;
		} else if (resCpuWrite) {
			d3dUsage = D3D11_USAGE_DYNAMIC;
			_resUsage = Usage::CPU_WRITE;
		} else if (((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) || dataSize < resSize) {
			d3dUsage = D3D11_USAGE_DEFAULT;
			_resUsage = Usage::GPU_WRITE;
		} else if (dataSize >= resSize) {
			d3dUsage = D3D11_USAGE_IMMUTABLE;
		} else {
			d3dUsage = D3D11_USAGE_DEFAULT;
			_resUsage = Usage::GPU_WRITE;
		}
	}

	Usage BaseResource::map(Graphics* graphics, Usage mapUsage) {
		Usage ret = Usage::NONE;

		mapUsage &= Usage::CPU_READ | Usage::CPU_WRITE;
		if (handle && mapUsage != Usage::NONE) {
			unmap(graphics);

			UINT mapType = 0;
			if (((mapUsage & Usage::CPU_READ) == Usage::CPU_READ) && ((_resUsage & Usage::CPU_READ) == Usage::CPU_READ)) {
				mapType |= D3D11_MAP_READ;
				ret |= Usage::CPU_READ;
			} else {
				mapUsage &= ~Usage::CPU_READ;
			}
			if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
				if ((_resUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
					ret |= Usage::CPU_WRITE;
					if (_bindType == D3D11_BIND_CONSTANT_BUFFER) {
						if (graphics->getFeatureOptions().MapNoOverwriteOnDynamicConstantBuffer) {
							mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
							ret |= Usage::CPU_WRITE_NO_OVERWRITE;
						} else {
							mapType |= D3D11_MAP_WRITE_DISCARD;
							ret |= Usage::CPU_WRITE_DISCARD;
						}
					} else if (_bindType & (D3D11_BIND_VERTEX_BUFFER & D3D11_BIND_INDEX_BUFFER)) {
						mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
						ret |= Usage::CPU_WRITE_NO_OVERWRITE;
					} else {
						mapType |= D3D11_MAP_WRITE_DISCARD;
						ret |= Usage::CPU_WRITE_DISCARD;
					}
				} else if (((_resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) && ret != Usage::NONE) {
					ret |= Usage::GPU_WRITE;
				}
			} else {
				mapUsage &= ~Usage::CPU_WRITE;
			}

			if (mapType) {
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

				if (SUCCEEDED(graphics->getContext()->Map(handle, 0, (D3D11_MAP)mapType, 0, &mappedResource))) {
					_mapUsage = mapUsage;
					_mapData = mappedResource.pData;
				} else {
					ret = (_resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE ? Usage::GPU_WRITE : Usage::NONE;
				}
			}
		}

		return ret;
	}

	void BaseResource::unmap(Graphics* graphics) {
		if (_mapUsage != Usage::NONE) {
			graphics->getContext()->Unmap(handle, 0);
			_mapData = nullptr;
			_mapUsage = Usage::NONE;
		}
	}

	i32 BaseResource::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		if ((_mapUsage & Usage::CPU_READ) == Usage::CPU_READ) {
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
		if ((_mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)_mapData + offset, data, length);
				return length;
			}
			return 0;
		} else if ((_resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
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
		if (handle && (_resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {

		}
	}

	void BaseResource::releaseRes(Graphics* graphics) {
		if (handle) {
			unmap(graphics);

			handle->Release();
			handle = nullptr;
		}
		size = 0;
		_resUsage = Usage::NONE;
	}

	void BaseResource::_waitServerSync() {
	}

	void BaseResource::_delSync() {
	}
}