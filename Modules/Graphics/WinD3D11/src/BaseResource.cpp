#include "BaseResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseResource::BaseResource(UINT resType) :
		size(0),
		resType(resType),
		bindType(0),
		resUsage(Usage::NONE),
		handle(nullptr) {
	}

	BaseResource::~BaseResource() {
	}

	bool BaseResource::createInit(Graphics& graphics, Usage resUsage, uint32_t resSize, uint32_t dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
		cpuUsage = 0;

		bool resCpuRead = (resUsage & Usage::MAP_READ) == Usage::MAP_READ;
		bool resCpuWrite = (resUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE;

		if (resCpuRead) {
			if ((resUsage & Usage::IGNORE_UNSUPPORTED) == Usage::NONE && (resUsage & Usage::UPDATE) == Usage::UPDATE) {
				graphics.error("d3d11 Resource:create error : not support Usage::UPDATE");
				return false;
			}

			d3dUsage = D3D11_USAGE_STAGING;
			this->resUsage = Usage::MAP_READ;
			cpuUsage = D3D11_CPU_ACCESS_READ;
			if (resCpuWrite) {
				this->resUsage |= Usage::MAP_WRITE;
				cpuUsage |= D3D11_CPU_ACCESS_WRITE;
			}
		} else if (resCpuWrite && mipLevels <= 1) {
			if ((resUsage & Usage::IGNORE_UNSUPPORTED) == Usage::NONE && (resUsage & Usage::UPDATE) == Usage::UPDATE) {
				graphics.error("d3d11 Resource::create error : not support Usage::UPDATE");
				return false;
			}

			d3dUsage = D3D11_USAGE_DYNAMIC;
			this->resUsage = Usage::MAP_WRITE;
			cpuUsage = D3D11_CPU_ACCESS_WRITE;
		} else if (((resUsage & Usage::UPDATE) == Usage::UPDATE) || dataSize < resSize) {
			d3dUsage = D3D11_USAGE_DEFAULT;
			this->resUsage = Usage::UPDATE;
		} else if (dataSize >= resSize) {
			d3dUsage = D3D11_USAGE_IMMUTABLE;
		} else {
			this->resUsage = Usage::NONE;
			return false;
			//d3dUsage = D3D11_USAGE_DEFAULT;
			//this->resUsage = Usage::UPDATE;
		}

		if (d3dUsage != D3D11_USAGE_STAGING) bindType = resType;

		return true;
	}

	Usage BaseResource::map(Graphics& graphics, Usage expectMapUsage, Usage& mapUsage, UINT subresource, D3D11_MAPPED_SUBRESOURCE& mappedRes) {
		Usage ret = Usage::NONE;

		if (handle) {
			expectMapUsage &= resUsage & Usage::MAP_READ_WRITE;

			if (expectMapUsage == Usage::NONE) {
				unmap(graphics, mapUsage, subresource);
			} else {
				UINT mapType = 0;
				if ((expectMapUsage & Usage::MAP_READ) == Usage::MAP_READ) {
					mapType |= D3D11_MAP_READ;
					ret |= Usage::MAP_READ;
				}

				if ((expectMapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
					ret |= Usage::MAP_WRITE;
					if (bindType == D3D11_BIND_CONSTANT_BUFFER) {
						if (graphics.getInternalFeatures().MapNoOverwriteOnDynamicConstantBuffer) {
							mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
						} else {
							mapType |= D3D11_MAP_WRITE_DISCARD;
							ret |= Usage::DISCARD;
						}
					} else if (bindType & (D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER)) {
						mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
					} else {
						mapType |= D3D11_MAP_WRITE_DISCARD;
						ret |= Usage::DISCARD;
					}
				}

				if (mapUsage != expectMapUsage) {
					unmap(graphics, mapUsage, subresource);
					if (SUCCEEDED(graphics.getContext()->Map(handle, subresource, (D3D11_MAP)mapType, 0, &mappedRes))) {
						mapUsage = expectMapUsage;
					} else {
						ret = Usage::NONE;
					}
				}
			}
		}

		return ret;
	}

	void BaseResource::unmap(Graphics& graphics, Usage& mapUsage, UINT subresource) {
		if ((mapUsage & Usage::MAP_READ_WRITE) != Usage::NONE) {
			graphics.getContext()->Unmap(handle, subresource);
		}
		mapUsage = Usage::NONE;
	}

	void BaseResource::releaseRes() {
		if (handle) {
			handle->Release();
			handle = nullptr;
		}
		size = 0;
		bindType = 0;
		resUsage = Usage::NONE;
	}
}