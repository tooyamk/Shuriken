#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL DepthStencilState : public IDepthStencilState {
	public:
		DepthStencilState(Graphics& graphics, bool isInternal);
		virtual ~DepthStencilState();

		virtual const void* SRK_CALL getNative() const override;

		virtual const DepthState& SRK_CALL getDepthState() const override;
		virtual void SRK_CALL setDepthState(const DepthState& depthState) override;

		virtual const StencilState& SRK_CALL getStencilState() const override;
		virtual void SRK_CALL setStencilState(const StencilState& stencilState) override;

		inline const VkPipelineDepthStencilStateCreateInfo& SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline const DepthStencilFeature& SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		void SRK_CALL update();

	protected:
		bool _isInternal;
		bool _dirty;
		DepthState _depthState;
		StencilState _stencilState;
		VkPipelineDepthStencilStateCreateInfo _internalState;
		DepthStencilFeature _featureValue;

		void SRK_CALL _updateDepth();
		void SRK_CALL _updateStencil();
		void SRK_CALL _updateStecnilFace(VkStencilOpState& vkState, const StencilFaceState& state);
	};
}