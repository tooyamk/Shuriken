#include "BaseResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseResource::BaseResource(UINT resType) :
		size(0),
		resType(resType),
		bindType(0),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE),
		handle(nullptr) {
	}

	BaseResource::~BaseResource() {
	}

	void BaseResource::calcAllocateUsage(Usage resUsage, ui32 resSize, ui32 dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
		cpuUsage = 0;
		
		bool resCpuRead = (resUsage & Usage::CPU_READ) == Usage::CPU_READ;
		bool resCpuWrite = (resUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE;

		if (resCpuRead || ((resUsage & Usage::CPU_GPU_WRITE) == Usage::CPU_GPU_WRITE)) {
			d3dUsage = D3D11_USAGE_STAGING;
			this->resUsage = Usage::CPU_READ_WRITE | Usage::GPU_WRITE;
			cpuUsage = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		} else if (resCpuWrite && mipLevels <= 1) {
			d3dUsage = D3D11_USAGE_DYNAMIC;
			this->resUsage = Usage::CPU_WRITE;
			cpuUsage = D3D11_CPU_ACCESS_WRITE;
		} else if (((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) || dataSize < resSize) {
			d3dUsage = D3D11_USAGE_DEFAULT;
			this->resUsage = Usage::GPU_WRITE;
		} else if (dataSize >= resSize) {
			d3dUsage = D3D11_USAGE_IMMUTABLE;
		} else {
			d3dUsage = D3D11_USAGE_DEFAULT;
			this->resUsage = Usage::GPU_WRITE;
		}

		if (d3dUsage != D3D11_USAGE_STAGING) bindType = resType;
	}

	Usage BaseResource::map(Graphics* graphics, Usage mapUsage) {
		Usage ret = Usage::NONE;

		mapUsage &= Usage::CPU_READ | Usage::CPU_WRITE;
		if (handle && mapUsage != Usage::NONE) {
			unmap(graphics);

			UINT mapType = 0;
			if (((mapUsage & Usage::CPU_READ) == Usage::CPU_READ) && ((resUsage & Usage::CPU_READ) == Usage::CPU_READ)) {
				mapType |= D3D11_MAP_READ;
				ret |= Usage::CPU_READ;
			} else {
				mapUsage &= ~Usage::CPU_READ;
			}
			if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
				if ((resUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE && ((resUsage & (Usage::CPU_READ | Usage::GPU_WRITE)) == Usage::NONE)) {
					ret |= Usage::CPU_WRITE;
					if (bindType == D3D11_BIND_CONSTANT_BUFFER) {
						if (graphics->getFeatureOptions().MapNoOverwriteOnDynamicConstantBuffer) {
							mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
							ret |= Usage::CPU_WRITE_NO_OVERWRITE;
						} else {
							mapType |= D3D11_MAP_WRITE_DISCARD;
							ret |= Usage::CPU_WRITE_DISCARD;
						}
					} else if (bindType & (D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER)) {
						mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
						ret |= Usage::CPU_WRITE_NO_OVERWRITE;
					} else {
						mapType |= D3D11_MAP_WRITE_DISCARD;
						ret |= Usage::CPU_WRITE_DISCARD;
					}
				} else if ((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
					ret |= Usage::GPU_WRITE;
				}
			} else {
				mapUsage &= ~Usage::CPU_WRITE;
			}

			if (mapType) {
				if (SUCCEEDED(graphics->getContext()->Map(handle, 0, (D3D11_MAP)mapType, 0, &mappedRes))) {
					this->mapUsage = mapUsage;
				} else {
					ret = (resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE ? Usage::GPU_WRITE : Usage::NONE;
				}
			}
		}

		return ret;
	}

	void BaseResource::unmap(Graphics* graphics) {
		if (mapUsage != Usage::NONE) {
			graphics->getContext()->Unmap(handle, 0);
			mapUsage = Usage::NONE;
		}
	}

	void BaseResource::flush() {
		if (handle && (resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {

		}
	}

	void BaseResource::releaseRes(Graphics* graphics) {
		if (handle) {
			unmap(graphics);

			handle->Release();
			handle = nullptr;
		}
		size = 0;
		bindType = 0;
		resUsage = Usage::NONE;
	}

	void BaseResource::_waitServerSync() {
	}

	void BaseResource::_delSync() {
	}
}