#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL BaseBuffer {
	public:
		BaseBuffer(Graphics& graphics, UINT bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL _stroage(ui32 size, ui32 bufferUsage, const void* data = nullptr);
		bool AE_CALL _map(ui32 mapUsage);
		void AE_CALL _unmap();
		i32 AE_CALL _read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL _write(ui32 offset, const void* data, ui32 length);
		void AE_CALL _flush();

	protected:
		Graphics* _grap;
		ui32 _size;
		UINT _bufferType;
		ID3D11Buffer* _handle;
		ui32 _bufferUsage;

		ui32 _mapUsage;
		void* _mapData;

		void _delBuffer();
		void _waitServerSync();
		void _delSync();
	};
}