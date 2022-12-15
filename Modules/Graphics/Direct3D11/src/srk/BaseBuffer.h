#pragma once

#include "BaseResource.h"

namespace srk::modules::graphics::d3d11 {
	class BaseBuffer : public BaseResource {
	public:
		BaseBuffer(UINT bufferType);
		virtual ~BaseBuffer();

		bool SRK_CALL create(Graphics& graphics, size_t size, Usage resUsage, const void* data = nullptr, size_t dataSize = 0);
		Usage SRK_CALL map(Graphics& graphics, Usage expectMapUsage);
		void SRK_CALL unmap(Graphics& graphics);
		size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
		size_t SRK_CALL write(const void* data, size_t length, size_t offset);
		size_t SRK_CALL update(Graphics& graphics, const void* data, size_t length, size_t offset);

		Usage mapUsage;
		D3D11_MAPPED_SUBRESOURCE mappedRes;

		void SRK_CALL releaseBuffer(Graphics& graphics);
	};
}