#include "Sampler.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	Sampler::Sampler(Graphics& graphics) : ISampler(graphics),
		_handle(0),
		_dirty(DirtyFlag::EMPTY) {
		memset(&_desc, 0, sizeof(_desc));
		_desc.LOD.set(-1000.f, 1000.f);
		_desc.LODBias = 0.f;
		_desc.maxAnisotropy = 1.0f;
		_desc.compareFunc = GL_NEVER;

		_updateFilter();
		_updateAddress();

		memcpy(&_oldDesc, &_desc, sizeof(_desc));
	}

	Sampler::~Sampler() {
		_releaseRes();
	}

	const void* Sampler::getNative() const {
		return this;
	}

	void Sampler::setFilter(const SamplerFilter& filter) {
		auto f = filter;
		if (f.operation != SamplerFilterOperation::COMPARISON) f.operation = SamplerFilterOperation::NORMAL;
		if (_filter != f) {
			_filter = f;

			_updateFilter();
			_setDirty(!memEqual<sizeof(_desc.filter)>(&_oldDesc.filter, &_desc.filter), DirtyFlag::FILTER);
		}
	}

	void Sampler::setComparisonFunc(ComparisonFunc func) {
		auto fn = Graphics::convertComparisonFunc(func);
		if (_desc.compareFunc != fn) {
			_desc.compareFunc = fn;

			_setDirty(_oldDesc.compareFunc != _desc.compareFunc, DirtyFlag::COMPARE_FUNC);
		}
	}

	void Sampler::setAddress(const SamplerAddress& address) {
		if (_address != address) {
			_address = address;

			_updateAddress();
			_setDirty(!memEqual<sizeof(_desc.address)>(&_oldDesc.address, &_desc.address), DirtyFlag::ADDRESS);
		}
	}

	void Sampler::setMipLOD(f32 min, f32 max) {
		if (_desc.LOD[0] != min || _desc.LOD[1] != max) {
			_desc.LOD.set(min, max);

			_setDirty(!memEqual<sizeof(_desc.LOD)>(&_oldDesc.LOD, &_desc.LOD), DirtyFlag::LOD);
		}
	}

	void Sampler::setMipLODBias(f32 bias) {
		if (_desc.LODBias != bias) {
			_desc.LODBias = bias;

			_setDirty(_oldDesc.LODBias != _desc.LODBias, DirtyFlag::LOD_BIAS);
		}
	}

	void Sampler::setMaxAnisotropy(uint32_t max) {
		if (auto& features = _graphics.get<Graphics>()->getInternalFeatures(); max > features.maxAnisotropy) max = features.maxAnisotropy;
		if (_desc.maxAnisotropy != max) {
			_desc.maxAnisotropy = max;

			_setDirty(_oldDesc.maxAnisotropy != _desc.maxAnisotropy, DirtyFlag::MAX_ANISOTROPY);
		}
	}

	void Sampler::setBorderColor(const Vec4f32& color) {
		if (_desc.borderColor != color) {
			_desc.borderColor.set(color);

			_setDirty(!memEqual<sizeof(_desc.borderColor)>(&_oldDesc.borderColor, &_desc.borderColor), DirtyFlag::BORDER_COLOR);
		}
	}

	void Sampler::_updateFilter() {
		_desc.filter.min = GL_NEAREST;
		_desc.filter.mag = GL_NEAREST;
		if (_filter.minification == SamplerFilterMode::ANISOTROPIC || _filter.magnification == SamplerFilterMode::ANISOTROPIC || _filter.mipmap == SamplerFilterMode::ANISOTROPIC) {
			//min = GL_TEXTURE_MAX_ANISOTROPY_EXT;
			//mag = GL_TEXTURE_MAX_ANISOTROPY_EXT;
		} else {
			if (_filter.minification == SamplerFilterMode::POINT) {
				_desc.filter.min = _filter.mipmap == SamplerFilterMode::POINT ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
			} else {
				_desc.filter.min = _filter.mipmap == SamplerFilterMode::POINT ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
			}
			_desc.filter.mag = _filter.magnification == SamplerFilterMode::POINT && _filter.mipmap == SamplerFilterMode::POINT ? GL_NEAREST : GL_LINEAR;
		}

		_desc.filter.compareMode = _filter.operation == SamplerFilterOperation::COMPARISON ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
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
		_desc.address.s = _convertAddressMode(_address.u);
		_desc.address.t = _convertAddressMode(_address.v);
		_desc.address.r = _convertAddressMode(_address.w);
	}

	void Sampler::update() {
		if (_dirty) {
			if (_dirty & DirtyFlag::EMPTY) {
				glGenSamplers(1, &_handle);
				_dirty = ~DirtyFlag::EMPTY;
			}

			if (_dirty & DirtyFlag::FILTER) {
				glSamplerParameteri(_handle, GL_TEXTURE_COMPARE_MODE, _desc.filter.compareMode);

				glSamplerParameteri(_handle, GL_TEXTURE_MIN_FILTER, _desc.filter.min);
				glSamplerParameteri(_handle, GL_TEXTURE_MAG_FILTER, _desc.filter.mag);
			}

			if (_dirty & DirtyFlag::COMPARE_FUNC) glSamplerParameteri(_handle, GL_TEXTURE_COMPARE_FUNC, _desc.compareFunc);

			if (_dirty & DirtyFlag::ADDRESS) {
				glSamplerParameteri(_handle, GL_TEXTURE_WRAP_S, _desc.address.s);
				glSamplerParameteri(_handle, GL_TEXTURE_WRAP_T, _desc.address.t);
				glSamplerParameteri(_handle, GL_TEXTURE_WRAP_R, _desc.address.r);
			}

			if (_dirty & DirtyFlag::LOD) {
				glSamplerParameterf(_handle, GL_TEXTURE_MIN_LOD, _desc.LOD[0]);
				glSamplerParameterf(_handle, GL_TEXTURE_MAX_LOD, _desc.LOD[1]);
			}

			if (_dirty & DirtyFlag::LOD_BIAS) glSamplerParameterf(_handle, GL_TEXTURE_LOD_BIAS, _desc.LODBias);
			if (_dirty & DirtyFlag::MAX_ANISOTROPY) glSamplerParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, _desc.maxAnisotropy);
			if (_dirty & DirtyFlag::BORDER_COLOR) glSamplerParameterfv(_handle, GL_TEXTURE_BORDER_COLOR, _desc.borderColor);

			memcpy(&_oldDesc, &_desc, sizeof(_desc));
			_dirty = 0;
		}
	}

	void Sampler::_releaseRes() {
		if (_handle) {
			glDeleteSamplers(1, &_handle);
			_handle = 0;
		}
		_dirty |= DirtyFlag::EMPTY;
	}
}