#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL RasterizerState : public IRasterizerState {
	public:
		RasterizerState(Graphics& graphics);
		virtual ~RasterizerState();

		virtual FillMode AE_CALL getFillMode() const override;
		virtual void AE_CALL setFillMode(FillMode fill) override;

		virtual CullMode AE_CALL getCullMode() const override;
		virtual void AE_CALL setCullMode(CullMode cull) override;

		virtual FrontFace AE_CALL getFrontFace() const override;
		virtual void AE_CALL setFrontFace(FrontFace front) override;

		inline const InternalRasterizerState& AE_CALL getInternalState() const {
			return _internalState;
		}

		inline const uint64_t& AE_CALL getFeatureValue() const {
			return _featureValue;
		}

		void AE_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType FILL_MODE = 0b1 << 1;
			static const DirtyType CULL_MODE = 0b1 << 2;
			static const DirtyType FRONT_FACE = 0b1 << 3;
		};

		struct Desc {
			FillMode fillMode;
			CullMode cullMode;
			FrontFace frontFace;
		};

		DirtyType _dirty;
		Desc _desc;
		Desc _oldDesc;
		InternalRasterizerState _internalState;
		uint64_t _featureValue;

		static GLenum AE_CALL _convertFillMode(FillMode mode);
		static GLenum AE_CALL _convertCullMode(CullMode mode);
		static GLenum AE_CALL _convertFrontFace(FrontFace front);

		inline void AE_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}