#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
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

		inline const VkPipelineColorBlendStateCreateInfo& SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline const uint64_t& SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		void SRK_CALL update();

	protected:
		bool _isInternal;
		bool _dirty;
		uint8_t _count;
		const uint8_t MAX_MRT_COUNT;
		std::vector<RenderTargetBlendState> _rtStatus;
		std::vector<VkPipelineColorBlendAttachmentState> _internalRtStatus;
		VkPipelineColorBlendStateCreateInfo _internalState;
		uint64_t _featureValue;

		static VkBlendFactor SRK_CALL _convertBlendFactor(BlendFactor factor);
		static VkBlendOp SRK_CALL _convertBlendOp(BlendOp op);

		void SRK_CALL _updateInternalState(uint8_t index);
	};
}