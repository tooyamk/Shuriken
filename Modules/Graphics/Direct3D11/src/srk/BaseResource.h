#pragma once

#include "Base.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	class BaseResource {
	public:
		BaseResource(D3D11_BIND_FLAG resType);
		virtual ~BaseResource();

		template<bool Tex>
		bool SRK_CALL createInit(Graphics& graphics, Usage resUsage, uint32_t resSize, uint32_t dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
			using namespace srk::enum_operators;

			Usage supportedUsages;
			if constexpr (Tex) {
				supportedUsages = graphics.getTexCreateUsageMask();
			} else {
				supportedUsages = graphics.getBufferCreateUsageMask();
			}

			if ((resUsage & Usage::IGNORE_UNSUPPORTED) == Usage::IGNORE_UNSUPPORTED) {
				resUsage &= supportedUsages;
			} else if (auto u = (resUsage & (~supportedUsages)); u != Usage::NONE) {
				graphics.error(std::format("D3D Resource::create error : has not support Usage {}", (std::underlying_type_t<Usage>)u));
				return false;
			}

			cpuUsage = 0;

			auto resCpuRead = (resUsage & Usage::MAP_READ) == Usage::MAP_READ;
			auto resCpuWrite = (resUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE;
			auto resGpuWrite = (resUsage & Usage::COPY_DST) == Usage::COPY_DST;

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
				if (resGpuWrite) this->resUsage |= Usage::COPY_DST;
			} else if (resCpuWrite && mipLevels <= 1) {
				if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
					graphics.error("D3D Resource::create error : could not enable Usage::UPDATE and Usage::MAP_WRITE at same time");
					return false;
				}

				this->resUsage = Usage::MAP_WRITE;
				cpuUsage = D3D11_CPU_ACCESS_WRITE;

				if ((resUsage & Usage::COPY_DST) == Usage::COPY_DST) {
					d3dUsage = D3D11_USAGE_STAGING;
					this->resUsage |= Usage::COPY_DST;
				} else {
					d3dUsage = D3D11_USAGE_DYNAMIC;
				}
			} else if ((resUsage & (Usage::UPDATE | Usage::COPY_DST)) != Usage::NONE) {
				d3dUsage = D3D11_USAGE_DEFAULT;
				this->resUsage = resUsage & (Usage::UPDATE | Usage::COPY_DST);
			} else if ((resUsage & Usage::RENDERABLE) == Usage::NONE) {
				d3dUsage = D3D11_USAGE_IMMUTABLE;
			} else {
				//this->resUsage = Usage::NONE;
				//return false;
				d3dUsage = D3D11_USAGE_DEFAULT;
				//this->resUsage = Usage::UPDATE;
			}

			if ((resUsage & Usage::COPY_SRC) == Usage::COPY_SRC) this->resUsage |= Usage::COPY_SRC;

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

		Usage SRK_CALL map(Graphics& graphics, Usage expectMapUsage, Usage& mapUsage, UINT subresource, D3D11_MAPPED_SUBRESOURCE& mappedRes);
		void SRK_CALL unmap(Graphics& graphics, Usage& mapUsage, UINT subresource);
		void SRK_CALL releaseRes();

		D3D11_BIND_FLAG resType;
		UINT bindType;
		Usage resUsage;
		size_t size;

		ID3D11Resource* handle;
	};
}