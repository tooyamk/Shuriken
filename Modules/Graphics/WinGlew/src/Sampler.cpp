#include "Sampler.h"
#include "Graphics.h"
#include "base/ByteArray.h"

namespace aurora::modules::graphics::win_glew {
	Sampler::Sampler(Graphics& graphics) : ISampler(graphics),
		_handle(0),
		_dirty(0xFF),
		_internalLOD({ -1000.f, 1000.f }),
		_internalLODBias(0.f),
		_maxAnisotropy(1.0f),
		_internalCompareFunc(GL_NEVER) {
		_updateFilter();
		_updateAddress();
	}

	Sampler::~Sampler() {
		_releaseRes();
	}

	void Sampler::setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) {
		if (op != SamplerFilterOperation::COMPARISON) op = SamplerFilterOperation::NORMAL;
		if (_filter.operation != op || _filter.minification != min || _filter.magnification != mag || _filter.mipmap != mipmap) {
			_filter.operation = op;
			_filter.minification = min;
			_filter.magnification = mag;
			_filter.mipmap = mipmap;

			_updateFilter();
			_dirty |= DirtyFlag::FILTER;
		}
	}

	void Sampler::setComparisonFunc(SamplerComparisonFunc func) {
		auto fn = _convertComparisonFunc(func);
		if (_internalCompareFunc != fn) {
			_internalCompareFunc = fn;
			_dirty |= DirtyFlag::COMPARE_FUNC;
		}
	}

	void Sampler::setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) {
		if (_address.u != u || _address.v != v || _address.w != w) {
			_address.u = u;
			_address.v = v;
			_address.w = w;

			_updateAddress();
			_dirty |= DirtyFlag::ADDRESS;
		}
	}

	void Sampler::setMipLOD(f32 min, f32 max) {
		if (_internalLOD.min != min || _internalLOD.max != max) {
			_internalLOD.min = min;
			_internalLOD.max = max;

			_dirty |= DirtyFlag::LOD;
		}
	}

	void Sampler::setMipLODBias(f32 bias) {
		if (_internalLODBias != bias) {
			_internalLODBias = bias;

			_dirty |= DirtyFlag::LOD_BAIS;
		}
	}

	void Sampler::setMaxAnisotropy(ui32 max) {
		if (auto& features = _graphics.get<Graphics>()->getInternalFeatures(); max > features.maxAnisotropy) max = features.maxAnisotropy;
		if (_maxAnisotropy != max) {
			_maxAnisotropy = max;

			_dirty |= DirtyFlag::MAX_ANISOTROPY;
		}
	}

	void Sampler::setBorderColor(const Vec4f32& color) {
		if (_internalBorderColor != color) {
			_internalBorderColor.set(color);
			_dirty |= DirtyFlag::BORDER_COLOR;
		}
	}

	void Sampler::_updateFilter() {
		_internalFilter.min = GL_NEAREST;
		_internalFilter.mag = GL_NEAREST;
		if (_filter.minification == SamplerFilterMode::ANISOTROPIC || _filter.magnification == SamplerFilterMode::ANISOTROPIC || _filter.mipmap == SamplerFilterMode::ANISOTROPIC) {
			//min = GL_TEXTURE_MAX_ANISOTROPY_EXT;
			//mag = GL_TEXTURE_MAX_ANISOTROPY_EXT;
		} else {
			if (_filter.minification == SamplerFilterMode::POINT) {
				_internalFilter.min = _filter.mipmap == SamplerFilterMode::POINT ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
			} else {
				_internalFilter.min = _filter.mipmap == SamplerFilterMode::POINT ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
			}
			_internalFilter.mag = _filter.magnification == SamplerFilterMode::POINT && _filter.mipmap == SamplerFilterMode::POINT ? GL_NEAREST : GL_LINEAR;
		}

		_internalFilter.compareMode = _filter.operation == SamplerFilterOperation::COMPARISON ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
	}

	GLenum Sampler::_convertComparisonFunc(SamplerComparisonFunc func) {
		switch (func) {
		case SamplerComparisonFunc::NEVER:
			return GL_NEVER;
		case SamplerComparisonFunc::LESS:
			return GL_LESS;
		case SamplerComparisonFunc::EQUAL:
			return GL_EQUAL;
		case SamplerComparisonFunc::LESS_EQUAL:
			return GL_LEQUAL;
		case SamplerComparisonFunc::GREATER:
			return GL_GREATER;
		case SamplerComparisonFunc::NOT_EQUAL:
			return GL_NOTEQUAL;
		case SamplerComparisonFunc::GREATER_EQUAL:
			return GL_GEQUAL;
		case SamplerComparisonFunc::ALWAYS:
			return GL_ALWAYS;
		default:
			return GL_NEVER;
		}
	}

	GLenum Sampler::_convertAddressMode(SamplerAddressMode mode) {
		switch (mode) {
		case SamplerAddressMode::WRAP:
			return GL_REPEAT;
		case SamplerAddressMode::MIRROR:
			return GL_MIRRORED_REPEAT;
		case SamplerAddressMode::CLAMP:
			return GL_CLAMP_TO_EDGE;
		case SamplerAddressMode::BORDER:
			return GL_CLAMP_TO_BORDER;
		case SamplerAddressMode::MIRROR_ONCE:
			return GL_MIRROR_CLAMP_TO_EDGE;
		default:
			return GL_REPEAT;
		}
	}

	void Sampler::_updateAddress() {
		_internalAddress.s = _convertAddressMode(_address.u);
		_internalAddress.t = _convertAddressMode(_address.v);
		_internalAddress.r = _convertAddressMode(_address.w);
	}

	void Sampler::update() {
		if (_dirty) {
			if (!_handle) glGenSamplers(1, &_handle);

			if (_dirty & DirtyFlag::FILTER) {
				glSamplerParameteri(_handle, GL_TEXTURE_COMPARE_MODE, _internalFilter.compareMode);

				glSamplerParameteri(_handle, GL_TEXTURE_MIN_FILTER, _internalFilter.min);
				glSamplerParameteri(_handle, GL_TEXTURE_MAG_FILTER, _internalFilter.mag);
			}

			if (_dirty & DirtyFlag::COMPARE_FUNC) glSamplerParameteri(_handle, GL_TEXTURE_COMPARE_FUNC, _internalCompareFunc);

			if (_dirty & DirtyFlag::ADDRESS) {
				glSamplerParameteri(_handle, GL_TEXTURE_WRAP_S, _internalAddress.s);
				glSamplerParameteri(_handle, GL_TEXTURE_WRAP_S, _internalAddress.t);
				glSamplerParameteri(_handle, GL_TEXTURE_WRAP_S, _internalAddress.r);
			}

			if (_dirty & DirtyFlag::LOD) {
				glSamplerParameterf(_handle, GL_TEXTURE_MIN_LOD, _internalLOD.min);
				glSamplerParameterf(_handle, GL_TEXTURE_MAX_LOD, _internalLOD.max);
			}

			if (_dirty & DirtyFlag::LOD_BAIS) glSamplerParameterf(_handle, GL_TEXTURE_LOD_BIAS, _internalLODBias);
			if (_dirty & DirtyFlag::MAX_ANISOTROPY) glSamplerParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, _maxAnisotropy);
			if (_dirty & DirtyFlag::BORDER_COLOR) glSamplerParameterfv(_handle, GL_TEXTURE_BORDER_COLOR, _internalBorderColor);

			_dirty = 0;
		}
	}

	void Sampler::_releaseRes() {
		if (_handle) {
			glDeleteSamplers(1, &_handle);
			_handle = 0;
		}
		_dirty = 0xFF;
	}
}