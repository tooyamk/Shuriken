#pragma once

#include "BaseBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL ConstantBuffer : public IConstantBuffer {
	public:
		ConstantBuffer(Graphics& graphics);
		virtual ~ConstantBuffer();

		ui32* recordUpdateIds;

		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual i32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL flush() override;

		template<ProgramStage stage>
		inline void AE_CALL use(UINT slot) {
		}

		template<>
		inline void AE_CALL use<ProgramStage::VS>(UINT slot) {
			_graphics.get<Graphics>()->getContext()->VSSetConstantBuffers(slot, 1, (ID3D11Buffer**)&_baseBuffer.handle);
		}

		template<>
		inline void AE_CALL use<ProgramStage::PS>(UINT slot) {
			_graphics.get<Graphics>()->getContext()->PSSetConstantBuffers(slot, 1, (ID3D11Buffer**)&_baseBuffer.handle);
		}

	protected:
		BaseBuffer _baseBuffer;
	};
}