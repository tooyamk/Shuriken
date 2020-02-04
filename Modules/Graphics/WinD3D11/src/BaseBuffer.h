#pragma once

#include "BaseResource.h"

namespace aurora::modules::graphics::win_d3d11 {
	class BaseBuffer : public BaseResource {
	public:
		BaseBuffer(UINT bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL create(Graphics& graphics, uint32_t size, Usage resUsage, const void* data = nullptr, uint32_t dataSize = 0);
		Usage AE_CALL map(Graphics& graphics, Usage expectMapUsage);
		void AE_CALL unmap(Graphics& graphics);
		uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen);
		uint32_t AE_CALL write(Graphics& graphics, uint32_t offset, const void* data, uint32_t length);
		uint32_t AE_CALL update(Graphics& graphics, uint32_t offset, const void* data, uint32_t length);

		Usage mapUsage;
		D3D11_MAPPED_SUBRESOURCE mappedRes;

		void AE_CALL releaseBuffer(Graphics& graphics);
	};
}