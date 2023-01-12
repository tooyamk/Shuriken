#pragma once

#include "BaseResource.h"

namespace srk::modules::graphics::d3d11 {
	class BaseBuffer : public BaseResource {
	public:
		BaseBuffer(D3D11_BIND_FLAG bufferType);
		virtual ~BaseBuffer();

		bool SRK_CALL create(Graphics& graphics, size_t size, Usage requiredUsage, Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0);
		Usage SRK_CALL map(Graphics& graphics, Usage expectMapUsage);
		void SRK_CALL unmap(Graphics& graphics);
		size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
		size_t SRK_CALL write(const void* data, size_t length, size_t offset);
		size_t SRK_CALL update(Graphics& graphics, const void* data, size_t length, size_t offset);
		size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange);
		size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange);

		Usage mapUsage;
		D3D11_MAPPED_SUBRESOURCE mappedRes;
		void SRK_CALL releaseBuffer(Graphics& graphics);
	};
}