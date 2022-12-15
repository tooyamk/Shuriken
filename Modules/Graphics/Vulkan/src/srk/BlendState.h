#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual const void* SRK_CALL getNative() const override;
		virtual const Vec4f32& SRK_CALL getConstants() const override;
		virtual void SRK_CALL setConstants(const Vec4f32& val) override;
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

		void SRK_CALL _updateInternalState(uint8_t index);
	};
}