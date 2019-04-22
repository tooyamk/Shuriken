#pragma once

#include "BaseResource.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL BaseBuffer : public BaseResource {
	public:
		BaseBuffer(UINT bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL allocate(Graphics* graphics, ui32 size, ui32 resUsage, const void* data = nullptr);
	};
}