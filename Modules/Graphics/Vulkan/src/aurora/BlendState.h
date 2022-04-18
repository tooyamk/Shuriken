#pragma once

#include "Base.h"

namespace aurora::modules::graphics::vulkan {
	class Graphics;

	class AE_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual const void* AE_CALL getNative() const override;
		virtual uint8_t AE_CALL getCount() const override;
		virtual void AE_CALL setCount(uint8_t count) override;

		virtual const RenderTargetBlendState* AE_CALL getRenderTargetState(uint8_t index) const override;
		virtual bool AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

		inline const VkPipelineColorBlendStateCreateInfo& AE_CALL getInternalState() const {
			return _internalState;
		}

		inline const uint64_t& AE_CALL getFeatureValue() const {
			return _featureValue;
		}

		void AE_CALL update();

	protected:
		bool _isInternal;
		bool _dirty;
		uint8_t _count;
		const uint8_t MAX_MRT_COUNT;
		std::vector<RenderTargetBlendState> _rtStatus;
		std::vector<VkPipelineColorBlendAttachmentState> _internalRtStatus;
		VkPipelineColorBlendStateCreateInfo _internalState;
		uint64_t _featureValue;

		static VkBlendFactor AE_CALL _convertBlendFactor(BlendFactor factor);
		static VkBlendOp AE_CALL _convertBlendOp(BlendOp op);

		void AE_CALL _updateInternalState(uint8_t index);
	};
}