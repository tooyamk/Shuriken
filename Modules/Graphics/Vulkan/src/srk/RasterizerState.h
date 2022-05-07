#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class Graphics;

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

		inline const VkPipelineRasterizationStateCreateInfo& SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline uint32_t SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		void SRK_CALL update();

	protected:
		bool _isInternal;
		bool _dirty;

		FillMode _fillMode;
		CullMode _cullMode;
		FrontFace _frontFace;
		VkPipelineRasterizationStateCreateInfo _internalState;
		uint32_t _featureValue;

		static VkPolygonMode SRK_CALL _convertFillMode(FillMode mode);
		static VkCullModeFlagBits SRK_CALL _convertCullMode(CullMode mode);
		static VkFrontFace SRK_CALL _convertFrontFace(FrontFace mode);
	};
}