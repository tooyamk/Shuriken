#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL RasterizerState : public IRasterizerState {
	public:
		RasterizerState(Graphics& graphics, bool isInternal);
		virtual ~RasterizerState();

		virtual const void* SRK_CALL getNative() const override;

		virtual FillMode SRK_CALL getFillMode() const override;
		virtual void SRK_CALL setFillMode(FillMode fill) override;

		virtual CullMode SRK_CALL getCullMode() const override;
		virtual void SRK_CALL setCullMode(CullMode cull) override;

		virtual FrontFace SRK_CALL getFrontFace() const override;
		virtual void SRK_CALL setFrontFace(FrontFace front) override;

		virtual bool SRK_CALL getScissorEnabled() const override;
		virtual void SRK_CALL setScissorEnabled(bool enabled) override;

		inline const VkPipelineRasterizationStateCreateInfo& SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline const RasterizerFeature& SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		void SRK_CALL update();

	protected:
		bool _isInternal;
		bool _dirty;

		RasterizerDescriptor _desc;
		VkPipelineRasterizationStateCreateInfo _internalState;
		RasterizerFeature _featureValue;
	};
}