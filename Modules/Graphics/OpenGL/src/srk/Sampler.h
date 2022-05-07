#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class Graphics;

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

		inline GLuint SRK_CALL getInternalSampler() const {
			return _handle;
		}

		void SRK_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType FILTER = 0b1 << 1;
			static const DirtyType COMPARE_FUNC = 0b1 << 2;
			static const DirtyType ADDRESS = 0b1 << 3;
			static const DirtyType LOD = 0b1 << 4;
			static const DirtyType LOD_BIAS = 0b1 << 5;
			static const DirtyType MAX_ANISOTROPY = 0b1 << 6;
			static const DirtyType BORDER_COLOR = 0b1 << 7;
		};


		struct Desc {
			struct {
				GLenum compareMode;
				GLenum min;
				GLenum mag;
			} filter;

			struct {
				GLenum s;
				GLenum t;
				GLenum r;
			} address;

			GLenum compareFunc;
			Vec2<GLfloat> LOD;
			GLfloat LODBias;
			GLfloat maxAnisotropy;
			Vec4<GLfloat> borderColor;
		};


		DirtyType _dirty;
		GLuint _handle;
		SamplerFilter _filter;
		SamplerAddress _address;

		Desc _desc;
		Desc _oldDesc;

		void SRK_CALL _updateFilter();

		static GLenum SRK_CALL _convertAddressMode(SamplerAddressMode mode);
		void SRK_CALL _updateAddress();

		void SRK_CALL _releaseRes();

		inline void SRK_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}