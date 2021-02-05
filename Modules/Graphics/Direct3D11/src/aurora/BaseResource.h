#pragma once

#include "Base.h"
#include "Graphics.h"

namespace aurora::modules::graphics::d3d11 {
	class BaseResource {
	public:
		BaseResource(UINT resType);
		virtual ~BaseResource();

		template<bool Tex>
		bool AE_CALL createInit(Graphics& graphics, Usage resUsage, uint32_t resSize, uint32_t dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
			if constexpr (!Tex) {
				if ((resUsage & Usage::RENDERABLE) == Usage::RENDERABLE) {
					if ((resUsage & Usage::IGNORE_UNSUPPORTED) == Usage::NONE) {
						graphics.error("D3D Resource::create error : not support Usage::RENDERABLE");
						return false;
					} else {
						resUsage &= ~Usage::RENDERABLE;
					}
				}
			}

			cpuUsage = 0;

			bool resCpuRead = (resUsage & Usage::MAP_READ) == Usage::MAP_READ;
			bool resCpuWrite = (resUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE;

			if (resCpuRead) {
				if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
					graphics.error("D3D Resource::create error : could not enable Usage::UPDATE and Usage::MAP_READ at same time");
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
				if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
					graphics.error("D3D Resource::create error : could not enable Usage::UPDATE and Usage::MAP_WRITE at same time");
					return false;
				}

				d3dUsage = D3D11_USAGE_DYNAMIC;
				this->resUsage = Usage::MAP_WRITE;
				cpuUsage = D3D11_CPU_ACCESS_WRITE;
			} else if (((resUsage & Usage::UPDATE) == Usage::UPDATE) || dataSize < resSize) {
				d3dUsage = D3D11_USAGE_DEFAULT;
				this->resUsage = Usage::UPDATE;
			} else if (dataSize >= resSize && (resUsage & Usage::RENDERABLE) == Usage::NONE) {
				d3dUsage = D3D11_USAGE_IMMUTABLE;
			} else {
				//this->resUsage = Usage::NONE;
				//return false;
				d3dUsage = D3D11_USAGE_DEFAULT;
				//this->resUsage = Usage::UPDATE;
			}

			bindType = 0;

			if constexpr (Tex) {
				if ((resUsage & Usage::RENDERABLE) == Usage::RENDERABLE) {
					if (d3dUsage == D3D11_USAGE_DEFAULT) {
						this->resUsage |= Usage::RENDERABLE;
						bindType |= D3D11_BIND_RENDER_TARGET;
					} else {
						if (resCpuRead) {
							if (resCpuWrite) {
								graphics.error("D3D Texture::create error : could not enable Usage::RENDERABLE and Usage::MAP_READ_WRITE at same time");
							} else {
								graphics.error("D3D Texture::create error : could not enable Usage::RENDERABLE and Usage::MAP_READ at same time");
							}
						} else if (resCpuWrite) {
							graphics.error("D3D Texture::create error : could not enable Usage::RENDERABLE and Usage::MAP_WRITE at same time");
						} else {
							graphics.error("D3D Texture::create error : not support Usage::RENDERABLE, internal usage type error");
						}
						return false;
					}
				}
			}

			if (d3dUsage != D3D11_USAGE_STAGING) bindType |= resType;

			return true;
		}
		Usage AE_CALL map(Graphics& graphics, Usage expectMapUsage, Usage& mapUsage, UINT subresource, D3D11_MAPPED_SUBRESOURCE& mappedRes);
		void AE_CALL unmap(Graphics& graphics, Usage& mapUsage, UINT subresource);
		void AE_CALL releaseRes();

		UINT resType;
		UINT bindType;
		Usage resUsage;
		size_t size;

		ID3D11Resource* handle;
	};
}