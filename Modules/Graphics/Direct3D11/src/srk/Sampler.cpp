#include "Sampler.h"
#include "Graphics.h"
#include "srk/ByteArray.h"

namespace srk::modules::graphics::d3d11 {
	Sampler::Sampler(Graphics& graphics) : ISampler(graphics),
		_dirty(DirtyFlag::EMPTY),
		_samplerState(nullptr) {
		memset(&_desc, 0, sizeof(_desc));
		_updateFilter();
		_updateAddress();

		_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		_desc.MaxLOD = D3D11_FLOAT32_MAX;
		_oldDesc = _desc;
	}

	Sampler::~Sampler() {
		_releaseRes();
	}

	const void* Sampler::getNative() const {
		return this;
	}

	void Sampler::setFilter(const SamplerFilter& filter) {
		if (_filter != filter) {
			_filter = filter;

			_updateFilter();
			_setDirty(_oldDesc.Filter != _desc.Filter, DirtyFlag::FILTER);
		}
	}

	void Sampler::setComparisonFunc(ComparisonFunc func) {
		if (auto fn = Graphics::convertComparisonFunc(func); _desc.ComparisonFunc != fn) {
			_desc.ComparisonFunc = fn;
			_setDirty(_oldDesc.ComparisonFunc != _desc.ComparisonFunc, DirtyFlag::COMPARISON_FUNC);
		}
	}

	void Sampler::setAddress(const SamplerAddress& address) {
		if (_address != address) {
			_address = address;

			_updateAddress();
			_setDirty(_oldDesc.AddressU != _desc.AddressU || _oldDesc.AddressV != _desc.AddressV || _oldDesc.AddressW != _desc.AddressW, DirtyFlag::ADDRESS);
		}
	}

	void Sampler::setMipLOD(float32_t min, float32_t max) {
		if (_desc.MinLOD != min || _desc.MaxLOD != max) {
			_desc.MinLOD = min;
			_desc.MaxLOD = max;

			_setDirty(_oldDesc.MinLOD != _desc.MinLOD || _oldDesc.MaxLOD != _desc.MaxLOD, DirtyFlag::LOD);
		}
	}

	void Sampler::setMipLODBias(float32_t bias) {
		if (_desc.MipLODBias != bias) {
			_desc.MipLODBias = bias;

			_setDirty(_oldDesc.MipLODBias != _desc.MipLODBias, DirtyFlag::LOD_BIAS);
		}
	}

	void Sampler::setMaxAnisotropy(uint32_t max) {
		if (_desc.MaxAnisotropy != max) {
			_desc.MaxAnisotropy = max;
			
			_setDirty(_oldDesc.MaxAnisotropy != _desc.MaxAnisotropy, DirtyFlag::MAX_ANISOTROPY);
		}
	}

	void Sampler::setBorderColor(const Vec4f32& color) {
		if (!Math::equal(_desc.BorderColor, color.data)) {
			color.copyTo(_desc.BorderColor);

			_setDirty(memcmp(_oldDesc.BorderColor, _desc.BorderColor, sizeof(_desc.BorderColor)), DirtyFlag::BORDER_COLOR);
		}
	}

	void Sampler::_updateFilter() {
		uint32_t filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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

	void Sampler::update() {
		if (_dirty) {
			_releaseRes();
			if (SUCCEEDED(_graphics.get<Graphics>()->getDevice()->CreateSamplerState(&_desc, &_samplerState))) {
				_oldDesc = _desc;
				_dirty = 0;
			}
		}
	}

	void Sampler::_releaseRes() {
		if (_samplerState) {
			_samplerState->Release();
			_samplerState = nullptr;
		}
		_dirty |= DirtyFlag::EMPTY;
	}
}