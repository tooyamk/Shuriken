#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual const void* SRK_CALL getNative() const override;
		virtual uint8_t SRK_CALL getCount() const override;
		virtual void SRK_CALL setCount(uint8_t count) override;

		virtual const RenderTargetBlendState* SRK_CALL getRenderTargetState(uint8_t index) const override;
		virtual bool SRK_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

		inline const InternalRenderTargetBlendState& SRK_CALL getInternalRenderTargetState(uint8_t index) const {
			return _status[index];
		}

	protected:
		bool _isInternal;
		uint8_t _count;
		const uint8_t MAX_MRT_COUNT;
		std::vector<InternalRenderTargetBlendState> _status;

		void SRK_CALL _updateInternalState(uint8_t index);
	};
}