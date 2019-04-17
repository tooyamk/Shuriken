#pragma once

#include "BaseBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL ConstantBuffer : private BaseBuffer, public IConstantBuffer {
	public:
		ConstantBuffer(Graphics& graphics);
		virtual ~ConstantBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) override;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;

		template<ProgramStage stage>
		void AE_CALL use(UINT slot) {
		}

		template<>
		void AE_CALL use<ProgramStage::VS>(UINT slot) {
			((Graphics*)_graphics)->getContext()->VSSetConstantBuffers(slot, 1, &_handle);
		}

		template<>
		void AE_CALL use<ProgramStage::PS>(UINT slot) {
			((Graphics*)_graphics)->getContext()->PSSetConstantBuffers(slot, 1, &_handle);
		}

	protected:
		UINT _stride;
	};
}