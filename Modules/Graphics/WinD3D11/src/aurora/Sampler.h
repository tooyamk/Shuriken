#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL Sampler : public ISampler {
	public:
		Sampler(Graphics& graphics);
		virtual ~Sampler();

		virtual const void* AE_CALL getNative() const override;
		virtual void AE_CALL setFilter(const SamplerFilter& filter) override;
		virtual void AE_CALL setComparisonFunc(ComparisonFunc func) override;
		virtual void AE_CALL setAddress(const SamplerAddress& address) override;
		virtual void AE_CALL setMipLOD(float32_t min, float32_t max) override;
		virtual void AE_CALL setMipLODBias(float32_t bias) override;
		virtual void AE_CALL setMaxAnisotropy(uint32_t max) override;
		virtual void AE_CALL setBorderColor(const Vec4f32& color) override;

		inline ID3D11SamplerState* AE_CALL getInternalSampler() const {
			return _samplerState;
		}

		void AE_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType FILTER = 0b1 << 1;
			static const DirtyType COMPARISON_FUNC = 0b1 << 2;
			static const DirtyType ADDRESS = 0b1 << 3;
			static const DirtyType LOD = 0b1 << 4;
			static const DirtyType LOD_BIAS = 0b1 << 5;
			static const DirtyType MAX_ANISOTROPY = 0b1 << 6;
			static const DirtyType BORDER_COLOR = 0b1 << 7;
		};

		DirtyType _dirty;
		SamplerFilter _filter;
		SamplerAddress _address;
		D3D11_SAMPLER_DESC _desc;
		D3D11_SAMPLER_DESC _oldDesc;
		ID3D11SamplerState* _samplerState;

		void AE_CALL _updateFilter();

		
		static D3D11_TEXTURE_ADDRESS_MODE AE_CALL _convertAddressMode(SamplerAddressMode mode);
		void AE_CALL _updateAddress();

		void AE_CALL _releaseRes();

		inline void AE_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}