#include "Sampler.h"
#include "base/ByteArray.h"

namespace aurora::modules::graphics::win_d3d11 {
	Sampler::Sampler(Graphics& graphics) : ISampler(graphics),
		_dirty(true),
		_samplerState(nullptr) {
		memset(&_desc, 0, sizeof(_desc));
		_updateFilter();
		_updateAddress();
		_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		_desc.MaxLOD = D3D11_FLOAT32_MAX;
	}

	Sampler::~Sampler() {
		_releaseRes();
	}

	void Sampler::setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) {
		if (_filter.operation != op || _filter.minification != min || _filter.magnification != mag || _filter.mipmap != mipmap) {
			_filter.operation = op;
			_filter.minification = min;
			_filter.magnification = mag;
			_filter.mipmap = mipmap;

			_updateFilter();
			_dirty = true;
		}
	}

	void Sampler::setComparisonFunc(SamplerComparisonFunc func) {
		auto fn = _convertComparisonFunc(func);
		if (_desc.ComparisonFunc != fn) {
			_desc.ComparisonFunc = fn;
			_dirty = true;
		}
	}

	void Sampler::setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) {
		if (_address.u != u || _address.v != v || _address.w != w) {
			_address.u = u;
			_address.v = v;
			_address.w = w;

			_updateAddress();
			_dirty = true;
		}
	}

	void Sampler::setMipLOD(f32 min, f32 max, f32 bias) {
		if (_desc.MinLOD != min || _desc.MaxLOD != max || _desc.MipLODBias != bias) {
			_desc.MinLOD = min;
			_desc.MaxLOD = max;
			_desc.MipLODBias = bias;

			_dirty = true;
		}
	}

	void Sampler::setMaxAnisotropy(ui32 max) {
		if (_desc.MaxAnisotropy != max) {
			_desc.MaxAnisotropy = max;
			_dirty = true;
		}
	}

	void Sampler::setBorderColor(const Vector4& color) {
		if (!ByteArray::isEqual((i8*)_desc.BorderColor, sizeof(_desc.BorderColor), (const i8*)&color, sizeof(_desc.BorderColor))) {
			memcpy(_desc.BorderColor, &color, sizeof(_desc.BorderColor));
			_dirty = true;
		}
	}

	void Sampler::_updateFilter() {
		ui32 filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		if (_filter.minification == SamplerFilterMode::ANISOTROPIC || _filter.magnification == SamplerFilterMode::ANISOTROPIC || _filter.mipmap == SamplerFilterMode::ANISOTROPIC) {
			filter = D3D11_FILTER_ANISOTROPIC;
		} else {
			if (_filter.minification == SamplerFilterMode::LINEAR) filter |= D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			if (_filter.magnification == SamplerFilterMode::LINEAR) filter |= D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			if (_filter.mipmap == SamplerFilterMode::LINEAR) filter |= D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		}
		if (_filter.operation == SamplerFilterOperation::COMPARISON) filter |= D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		_desc.Filter = (D3D11_FILTER)filter;
	}

	D3D11_COMPARISON_FUNC Sampler::_convertComparisonFunc(SamplerComparisonFunc func) {
		switch (func) {
		case SamplerComparisonFunc::NEVER:
			return D3D11_COMPARISON_NEVER;
		case SamplerComparisonFunc::LESS:
			return D3D11_COMPARISON_LESS;
		case SamplerComparisonFunc::EQUAL:
			return D3D11_COMPARISON_EQUAL;
		case SamplerComparisonFunc::LESS_EQUAL:
			return D3D11_COMPARISON_LESS_EQUAL;
		case SamplerComparisonFunc::GREATER:
			return D3D11_COMPARISON_GREATER;
		case SamplerComparisonFunc::NOT_EQUAL:
			return D3D11_COMPARISON_NOT_EQUAL;
		case SamplerComparisonFunc::GREATER_EQUAL:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case SamplerComparisonFunc::ALWAYS:
			return D3D11_COMPARISON_ALWAYS;
		default:
			return D3D11_COMPARISON_NEVER;
		}
	}

	D3D11_TEXTURE_ADDRESS_MODE Sampler::_convertAddressMode(SamplerAddressMode mode) {
		switch (mode) {
		case SamplerAddressMode::WRAP:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case SamplerAddressMode::MIRROR:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case SamplerAddressMode::CLAMP:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case SamplerAddressMode::BORDER:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		case SamplerAddressMode::MIRROR_ONCE:
			return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		default:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	void Sampler::_updateAddress() {
		_desc.AddressU = _convertAddressMode(_address.u);
		_desc.AddressV = _convertAddressMode(_address.v);
		_desc.AddressW = _convertAddressMode(_address.w);
	}

	void Sampler::_update() {
		if (_dirty) {
			_releaseRes();
			if (SUCCEEDED(((Graphics*)_graphics)->getDevice()->CreateSamplerState(&_desc, &_samplerState))) _dirty = false;
		}
	}

	void Sampler::_releaseRes() {
		if (_samplerState) {
			_samplerState->Release();
			_samplerState = nullptr;
		}
		_dirty = true;
	}
}