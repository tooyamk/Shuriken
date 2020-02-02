#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class BaseResource {
	public:
		BaseResource(UINT resType);
		virtual ~BaseResource();

		bool AE_CALL createInit(Graphics& graphics, Usage resUsage, uint32_t resSize, uint32_t dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage);
		Usage AE_CALL map(Graphics& graphics, Usage expectMapUsage, Usage& mapUsage, UINT subresource, D3D11_MAPPED_SUBRESOURCE& mappedRes);
		void AE_CALL unmap(Graphics& graphics, Usage& mapUsage, UINT subresource);
		void releaseRes();

		uint32_t size;
		UINT resType;

		UINT bindType;
		
		Usage resUsage;

		ID3D11Resource* handle;
	};
}