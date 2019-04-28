#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class BaseResource {
	public:
		BaseResource(UINT resType);
		virtual ~BaseResource();

		void AE_CALL createInit(Usage resUsage, ui32 resSize, ui32 dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage);
		Usage AE_CALL map(Graphics* graphics, Usage expectMapUsage, Usage& mapUsage, UINT subresource, D3D11_MAPPED_SUBRESOURCE& mappedRes);
		void AE_CALL unmap(Graphics* graphics, Usage& mapUsage, UINT subresource);
		void AE_CALL flush();

		ui32 size;
		UINT resType;

		UINT bindType;
		
		Usage resUsage;

		ID3D11Resource* handle;

		void releaseRes(Graphics* graphics);
		void _waitServerSync();
		void _delSync();
	};
}