#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL BaseResource {
	protected:
		BaseResource(Graphics& graphics, UINT bindType);
		virtual ~BaseResource();

		void AE_CALL _calcAllocateUsage(ui32 resUsage, const void* data, UINT& cpuUsage, D3D11_USAGE& d3dUsage);
		ui32 AE_CALL _map(ui32 mapUsage);
		void AE_CALL _unmap();
		i32 AE_CALL _read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL _write(ui32 offset, const void* data, ui32 length);
		void AE_CALL _flush();

		ui32 _size;
		UINT _bindType;
		
		ui32 _resUsage;

		ui32 _mapUsage;
		void* _mapData;

		Graphics* _grap;
		ID3D11Resource* _handle;

		void _delRes();
		void _waitServerSync();
		void _delSync();
	};
}