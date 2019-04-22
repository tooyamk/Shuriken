#include "Sampler.h"

namespace aurora::modules::graphics::win_d3d11 {
	Sampler::Sampler(Graphics& graphics) : ISampler(graphics),
		_dirty(true),
		_samplerState(nullptr) {
		memset(&_desc, 0, sizeof(_desc));
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
			_dirty = true;
		}
	}

	void Sampler::setFilter(const SamplerFilter& filter) {
		setFilter(filter.operation, filter.minification, filter.magnification, filter.mipmap);
	}

	void Sampler::_update() {
		if (_dirty) {
			_releaseRes();
			
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

			if (SUCCEEDED(((Graphics*)_graphics)->getDevice()->CreateSamplerState(&_desc, &_samplerState))) {
				_dirty = false;
			}
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