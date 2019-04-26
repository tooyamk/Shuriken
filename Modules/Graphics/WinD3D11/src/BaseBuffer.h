#pragma once

#include "BaseResource.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL BaseBuffer : public BaseResource {
	public:
		BaseBuffer(UINT bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL allocate(Graphics* graphics, ui32 size, Usage resUsage, const void* data = nullptr, ui32 dataSize = 0);
		i32 AE_CALL write(Graphics* graphics, ui32 offset, const void* data, ui32 length);
	};
}