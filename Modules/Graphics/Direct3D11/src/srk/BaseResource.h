#pragma once

#include "Base.h"
#include "Graphics.h"
#include "srk/String.h"

namespace srk::modules::graphics::d3d11 {
	class BaseResource {
	public:
		BaseResource(D3D11_BIND_FLAG resType);
		virtual ~BaseResource();

		template<bool Tex>
		bool SRK_CALL createInit(Graphics& graphics, Usage requiredUsage, Usage preferredUsage, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
			using namespace srk::enum_operators;

			Usage supportedUsages;
			if constexpr (Tex) {
				supportedUsages = graphics.getTexCreateUsageMask();
			} else {
				supportedUsages = graphics.getBufferCreateUsageMask();
			}

			if (auto u = (requiredUsage & (~supportedUsages)); u != Usage::NONE) {
				graphics.error("D3D Resource::create error : has not support requiredUsage " + String::toString((std::underlying_type_t<Usage>)u));
				return false;
			}

			auto allUsage = requiredUsage | preferredUsage;

			resUsage = Usage::NONE;
			bindType = 0;

			cpuUsage = 0;

			auto resCpuRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
			auto resCpuWrite = (allUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE;
			auto resGpuWrite = (allUsage & Usage::COPY_DST) == Usage::COPY_DST;

			if (resCpuRead) {
				if ((allUsage & Usage::UPDATE) == Usage::UPDATE) {
					auto rst = _createInitConflicted<Tex>(graphics, requiredUsage, preferredUsage, Usage::MAP_READ, Usage::UPDATE, mipLevels, cpuUsage, d3dUsage);
					if (rst) return *rst;

					graphics.error("D3D Resource::create error : could not enable Usage::UPDATE and Usage::MAP_READ at same time");
					return false;
				}

				d3dUsage = D3D11_USAGE_STAGING;
				resUsage = Usage::MAP_READ;
				cpuUsage = D3D11_CPU_ACCESS_READ;
				if (resCpuWrite) {
					resUsage |= Usage::MAP_WRITE;
					cpuUsage |= D3D11_CPU_ACCESS_WRITE;
				}
				if (resGpuWrite) resUsage |= Usage::COPY_DST;
			} else if (resCpuWrite && mipLevels <= 1) {
				if ((requiredUsage & Usage::UPDATE) == Usage::UPDATE) {
					auto rst = _createInitConflicted<Tex>(graphics, requiredUsage, preferredUsage, Usage::MAP_WRITE, Usage::UPDATE, mipLevels, cpuUsage, d3dUsage);
					if (rst) return *rst;

					graphics.error("D3D Resource::create error : could not enable Usage::UPDATE and Usage::MAP_WRITE at same time");
					return false;
				}

				resUsage = Usage::MAP_WRITE;
				cpuUsage = D3D11_CPU_ACCESS_WRITE;

				if ((allUsage & Usage::COPY_DST) == Usage::COPY_DST) {
					d3dUsage = D3D11_USAGE_STAGING;
					resUsage |= Usage::COPY_DST;
				} else {
					d3dUsage = D3D11_USAGE_DYNAMIC;
				}
			} else if ((allUsage & (Usage::UPDATE | Usage::COPY_DST)) != Usage::NONE) {
				d3dUsage = D3D11_USAGE_DEFAULT;
				resUsage = allUsage & (Usage::UPDATE | Usage::COPY_DST);
			} else if ((allUsage & Usage::RENDERABLE) == Usage::NONE) {
				d3dUsage = D3D11_USAGE_IMMUTABLE;
			} else {
				d3dUsage = D3D11_USAGE_DEFAULT;
			}

			if ((allUsage & Usage::COPY_SRC) == Usage::COPY_SRC) resUsage |= Usage::COPY_SRC;

			if constexpr (Tex) {
				if ((allUsage & Usage::RENDERABLE) == Usage::RENDERABLE) {
					if (d3dUsage == D3D11_USAGE_DEFAULT) {
						resUsage |= Usage::RENDERABLE;
						bindType |= D3D11_BIND_RENDER_TARGET;
					} else {
						if (resCpuRead) {
							if (resCpuWrite) {
								auto rst = _createInitConflicted<Tex>(graphics, requiredUsage, preferredUsage, Usage::RENDERABLE, Usage::MAP_READ_WRITE, mipLevels, cpuUsage, d3dUsage);
								if (rst) return *rst;

								graphics.error("D3D Texture::create error : could not enable Usage::RENDERABLE and Usage::MAP_READ_WRITE at same time");
							} else {
								auto rst = _createInitConflicted<Tex>(graphics, requiredUsage, preferredUsage, Usage::RENDERABLE, Usage::MAP_READ, mipLevels, cpuUsage, d3dUsage);
								if (rst) return *rst;

								graphics.error("D3D Texture::create error : could not enable Usage::RENDERABLE and Usage::MAP_READ at same time");
							}
						} else if (resCpuWrite) {
							auto rst = _createInitConflicted<Tex>(graphics, requiredUsage, preferredUsage, Usage::RENDERABLE, Usage::MAP_WRITE, mipLevels, cpuUsage, d3dUsage);
							if (rst) return *rst;

							graphics.error("D3D Texture::create error : could not enable Usage::RENDERABLE and Usage::MAP_WRITE at same time");
						} else {
							if ((requiredUsage & Usage::RENDERABLE) == Usage::RENDERABLE) {
								graphics.error("D3D Texture::create error : not support Usage::RENDERABLE, internal usage type error");
							} else {
								return createInit<Tex>(graphics, requiredUsage, preferredUsage & (~Usage::RENDERABLE), mipLevels, cpuUsage, d3dUsage);
							}
						}
						return false;
					}
				}
			}

			if ((resUsage & requiredUsage) != requiredUsage) {
				graphics.error("D3D Resource::create error : has not support requiredUsage " + String::toString((std::underlying_type_t<Usage>)(requiredUsage & (~(resUsage & requiredUsage)))));
				return false;
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

	private:
		template<bool Tex>
		std::optional<bool> SRK_CALL _createInitConflicted(Graphics& graphics, Usage requiredUsage, Usage preferredUsage, Usage conflictedUsage1, Usage conflictedUsage2, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage) {
			using namespace srk::enum_operators;

			if ((requiredUsage & conflictedUsage1) != Usage::NONE) {
				if ((requiredUsage & conflictedUsage2) != Usage::NONE) {
					return std::nullopt;
				} else {
					return std::make_optional(createInit<Tex>(graphics, requiredUsage, preferredUsage & (~conflictedUsage2), mipLevels, cpuUsage, d3dUsage));
				}
			} else if ((requiredUsage & conflictedUsage2) != Usage::NONE) {
				return std::make_optional(createInit<Tex>(graphics, requiredUsage, preferredUsage & (~conflictedUsage1), mipLevels, cpuUsage, d3dUsage));
			} else {
				return std::make_optional(createInit<Tex>(graphics, requiredUsage, preferredUsage & (~conflictedUsage2), mipLevels, cpuUsage, d3dUsage));
			}
		}
	};
}