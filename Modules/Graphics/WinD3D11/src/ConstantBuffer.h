#pragma once

#include "BaseBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL ConstantBuffer : private BaseBuffer, public IConstantBuffer {
	public:
		ConstantBuffer(Graphics& graphics);
		virtual ~ConstantBuffer();

		ui32* recordUpdateIds;

		virtual bool AE_CALL allocate(ui32 size, ui32 bufferUsage, const void* data = nullptr) override;
		virtual ui32 AE_CALL map(ui32 mapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;

		template<ProgramStage stage>
		void AE_CALL use(UINT slot) {
		}

		template<>
		void AE_CALL use<ProgramStage::VS>(UINT slot) {
			((Graphics*)_graphics)->getContext()->VSSetConstantBuffers(slot, 1, (ID3D11Buffer**)&_handle);
		}

		template<>
		void AE_CALL use<ProgramStage::PS>(UINT slot) {
			((Graphics*)_graphics)->getContext()->PSSetConstantBuffers(slot, 1, (ID3D11Buffer**)&_handle);
		}

	protected:
		UINT _stride;
	};
}