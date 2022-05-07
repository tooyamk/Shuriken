#include "BaseResource.h"
#include "Graphics.h"
#include <algorithm>

namespace srk::modules::graphics::d3d11 {
	BaseResource::BaseResource(UINT resType) :
		size(0),
		resType(resType),
		bindType(0),
		resUsage(Usage::NONE),
		handle(nullptr) {
	}

	BaseResource::~BaseResource() {
	}

	Usage BaseResource::map(Graphics& graphics, Usage expectMapUsage, Usage& mapUsage, UINT subresource, D3D11_MAPPED_SUBRESOURCE& mappedRes) {
		using namespace srk::enum_operators;

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
		using namespace srk::enum_operators;

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