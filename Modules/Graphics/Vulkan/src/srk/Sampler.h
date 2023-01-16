#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class SRK_MODULE_DLL Sampler : public ISampler {
	public:
		Sampler(Graphics& graphics);
		virtual ~Sampler();

		virtual const void* SRK_CALL getNative() const override;
		virtual void SRK_CALL setFilter(const SamplerFilter& filter) override;
		virtual void SRK_CALL setComparisonFunc(ComparisonFunc func) override;
		virtual void SRK_CALL setAddress(const SamplerAddress& address) override;
		virtual void SRK_CALL setMipLOD(float32_t min, float32_t max) override;
		virtual void SRK_CALL setMipLODBias(float32_t bias) override;
		virtual void SRK_CALL setMaxAnisotropy(uint32_t max) override;
		virtual void SRK_CALL setBorderColor(const Vec4f32& color) override;

		inline VkSampler SRK_CALL getVkSampler() const {
			return _sampler;
		}

		void SRK_CALL update();

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

		VkSampler _sampler;
		VkSamplerCreateInfo _samplerCreateInfo;
		VkSamplerCustomBorderColorCreateInfoEXT _smplerCustomBorderColorCreateInfo;
		VkSamplerCreateInfo _oldSamplerCreateInfo;
		Vec4f32 _oldBorderColor;

		void SRK_CALL _updateFilter();
		void SRK_CALL _updateAddress();
		void SRK_CALL _release();

		inline void SRK_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}