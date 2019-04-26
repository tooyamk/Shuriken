#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL BaseResource {
	public:
		BaseResource(UINT resType);
		virtual ~BaseResource();

		void AE_CALL calcAllocateUsage(Usage resUsage, ui32 resSize, ui32 dataSize, UINT mipLevels, UINT& cpuUsage, D3D11_USAGE& d3dUsage);
		Usage AE_CALL map(Graphics* graphics, Usage mapUsage);
		void AE_CALL unmap(Graphics* graphics);
		void AE_CALL flush();

		ui32 size;
		UINT resType;

		UINT bindType;
		
		Usage resUsage;

		Usage mapUsage;
		D3D11_MAPPED_SUBRESOURCE mappedRes;

		ID3D11Resource* handle;

		void releaseRes(Graphics* graphics);
		void _waitServerSync();
		void _delSync();
	};
}