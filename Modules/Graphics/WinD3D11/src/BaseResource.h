#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL BaseResource {
	public:
		BaseResource(UINT bindType);
		virtual ~BaseResource();

		void AE_CALL calcAllocateUsage(ui32 resUsage, const void* data, UINT& cpuUsage, D3D11_USAGE& d3dUsage);
		ui32 AE_CALL map(Graphics* graphics, ui32 mapUsage);
		void AE_CALL unmap(Graphics* graphics);
		i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL write(Graphics* graphics, ui32 offset, const void* data, ui32 length);
		void AE_CALL flush();

		ui32 size;
		UINT _bindType;
		
		ui32 _resUsage;

		ui32 _mapUsage;
		void* _mapData;

		ID3D11Resource* handle;

		void releaseRes(Graphics* graphics);
		void _waitServerSync();
		void _delSync();
	};
}