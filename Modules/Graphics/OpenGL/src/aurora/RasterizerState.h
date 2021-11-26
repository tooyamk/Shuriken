#pragma once

#include "Base.h"

namespace aurora::modules::graphics::gl {
	class Graphics;

	class AE_MODULE_DLL RasterizerState : public IRasterizerState {
	public:
		RasterizerState(Graphics& graphics, bool isInternal);
		virtual ~RasterizerState();

		virtual const void* AE_CALL getNative() const override;

		virtual FillMode AE_CALL getFillMode() const override;
		virtual void AE_CALL setFillMode(FillMode fill) override;

		virtual CullMode AE_CALL getCullMode() const override;
		virtual void AE_CALL setCullMode(CullMode cull) override;

		virtual FrontFace AE_CALL getFrontFace() const override;
		virtual void AE_CALL setFrontFace(FrontFace front) override;

		inline const InternalRasterizerState& AE_CALL getInternalState() const {
			return _internalState;
		}

		inline uint32_t AE_CALL getFeatureValue() const {
			return _featureValue;
		}

		void AE_CALL update();

	protected:
		bool _isInternal;
		bool _dirty;

		FillMode _fillMode;
		CullMode _cullMode;
		FrontFace _frontFace;
		InternalRasterizerState _internalState;
		uint32_t _featureValue;

		static GLenum AE_CALL _convertFillMode(FillMode mode);
		static GLenum AE_CALL _convertCullMode(CullMode mode);
		static GLenum AE_CALL _convertFrontFace(FrontFace front);
	};
}