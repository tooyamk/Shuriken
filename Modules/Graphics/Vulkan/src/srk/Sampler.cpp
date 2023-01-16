#include "Sampler.h"
#include "Graphics.h"
#include "srk/hash/xxHash.h"

namespace srk::modules::graphics::vulkan {
	Sampler::Sampler(Graphics& graphics) : ISampler(graphics),
		_dirty(DirtyFlag::EMPTY),
		_sampler(nullptr) {
		memset(&_samplerCreateInfo, 0, sizeof(_samplerCreateInfo));
		_samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		_updateAddress();

		if (graphics.getInternalFeatures().customBorderColor) {
			_samplerCreateInfo.pNext = &_smplerCustomBorderColorCreateInfo;

			memset(&_smplerCustomBorderColorCreateInfo, 0, sizeof(_smplerCustomBorderColorCreateInfo));
			_smplerCustomBorderColorCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
			_smplerCustomBorderColorCreateInfo.format = VK_FORMAT_UNDEFINED;
		}
		
		_samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		_samplerCreateInfo.maxAnisotropy = 1;
		_samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		_oldSamplerCreateInfo = _samplerCreateInfo;
	}

	Sampler::~Sampler() {
		_release();
	}

	const void* Sampler::getNative() const {
		return this;
	}

	void Sampler::setFilter(const SamplerFilter& filter) {
		if (_filter != filter) {
			_filter = filter;

			_updateFilter();
			_setDirty(_oldSamplerCreateInfo.minFilter != _samplerCreateInfo.minFilter || _oldSamplerCreateInfo.magFilter != _samplerCreateInfo.magFilter, DirtyFlag::FILTER);
		}
	}

	void Sampler::setComparisonFunc(ComparisonFunc func) {
		if (auto fn = Graphics::convertCompareOp(func); _samplerCreateInfo.compareOp != fn) {
			_samplerCreateInfo.compareOp = fn;

			_setDirty(_oldSamplerCreateInfo.compareOp != _samplerCreateInfo.compareOp, DirtyFlag::COMPARISON_FUNC);
		}
	}

	void Sampler::setAddress(const SamplerAddress& address) {
		if (_address != address) {
			_address = address;

			_updateAddress();
			_setDirty(_oldSamplerCreateInfo.addressModeU != _samplerCreateInfo.addressModeU || _oldSamplerCreateInfo.addressModeV != _samplerCreateInfo.addressModeV || _oldSamplerCreateInfo.addressModeW != _samplerCreateInfo.addressModeW, DirtyFlag::ADDRESS);
		}
	}

	void Sampler::setMipLOD(float32_t min, float32_t max) {
		if (_samplerCreateInfo.minLod != min || _samplerCreateInfo.maxLod != max) {
			_samplerCreateInfo.minLod = min;
			_samplerCreateInfo.maxLod = max;

			_setDirty(_oldSamplerCreateInfo.minLod != _samplerCreateInfo.minLod || _oldSamplerCreateInfo.maxLod != _samplerCreateInfo.maxLod, DirtyFlag::LOD);
		}
	}

	void Sampler::setMipLODBias(float32_t bias) {
		if (_samplerCreateInfo.mipLodBias != bias) {
			_samplerCreateInfo.mipLodBias = bias;

			_setDirty(_oldSamplerCreateInfo.mipLodBias != _samplerCreateInfo.mipLodBias, DirtyFlag::LOD_BIAS);
		}
	}

	void Sampler::setMaxAnisotropy(uint32_t max) {
		if ((decltype(max))_samplerCreateInfo.maxAnisotropy != max) {
			_samplerCreateInfo.maxAnisotropy = max;

			_setDirty(_oldSamplerCreateInfo.maxAnisotropy != _samplerCreateInfo.maxAnisotropy, DirtyFlag::MAX_ANISOTROPY);
		}
	}

	void Sampler::setBorderColor(const Vec4f32& color) {
		if (!Math::equal(_smplerCustomBorderColorCreateInfo.customBorderColor.float32, color.data)) {
			auto g = _graphics.get<Graphics>();
			if (g->getInternalFeatures().customBorderColor) {
				for (size_t i = 0; i < 4; ++i) _smplerCustomBorderColorCreateInfo.customBorderColor.float32[i] = color[i];

				_setDirty(!Math::equal(_oldBorderColor.data, _smplerCustomBorderColorCreateInfo.customBorderColor.float32), DirtyFlag::BORDER_COLOR);
			} else {
				auto c = VK_BORDER_COLOR_MAX_ENUM;
				if (color == Vec4f32::ZERO) {
					c = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
				} else if (color == Vec4f32(0.f, 0.f, 0.f, 1.f)) {
					c = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
				} else if (color == Vec4f32::ONE) {
					c = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				}

				if (c == VK_BORDER_COLOR_MAX_ENUM) {
					g->error("Vulkan Sampler::setBorderColor not suupprt custom color");
				} else {
					_samplerCreateInfo.borderColor = c;
					for (size_t i = 0; i < 4; ++i) _smplerCustomBorderColorCreateInfo.customBorderColor.float32[i] = color[i];

					_setDirty(!Math::equal(_oldBorderColor.data, _smplerCustomBorderColorCreateInfo.customBorderColor.float32), DirtyFlag::BORDER_COLOR);
				}
			}
		}
	}

	void Sampler::_updateFilter() {
		/*_samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		_samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		if (_filter.minification == SamplerFilterMode::ANISOTROPIC || _filter.magnification == SamplerFilterMode::ANISOTROPIC || _filter.mipmap == SamplerFilterMode::ANISOTROPIC) {
			_samplerCreateInfo.anisotropyEnable = VK_TRUE;
		} else {
			_samplerCreateInfo.anisotropyEnable = VK_FALSE;
			if (_filter.minification == SamplerFilterMode::LINEAR) _samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
			if (_filter.magnification == SamplerFilterMode::LINEAR) _samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
			_samplerCreateInfo.mipmapMode = _filter.mipmap == SamplerFilterMode::POINT ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}*/
		_samplerCreateInfo.minFilter = _filter.minification == SamplerFilterMode::POINT ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		_samplerCreateInfo.magFilter = _filter.magnification == SamplerFilterMode::POINT ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		_samplerCreateInfo.mipmapMode = _filter.mipmap == SamplerFilterMode::POINT ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
		_samplerCreateInfo.anisotropyEnable = _filter.minification == SamplerFilterMode::ANISOTROPIC || _filter.magnification == SamplerFilterMode::ANISOTROPIC || _filter.mipmap == SamplerFilterMode::ANISOTROPIC ? VK_TRUE : VK_FALSE;

		_samplerCreateInfo.compareEnable = _filter.operation == SamplerFilterOperation::COMPARISON ? VK_TRUE : VK_FALSE;
	}

	void Sampler::_updateAddress() {
		_samplerCreateInfo.addressModeU = Graphics::convertSamplerAddressMode(_address.u);
		_samplerCreateInfo.addressModeV = Graphics::convertSamplerAddressMode(_address.v);
		_samplerCreateInfo.addressModeW = Graphics::convertSamplerAddressMode(_address.w);
	}

	void Sampler::update() {
		if (_dirty) {
			_release();

			auto g = _graphics.get<Graphics>();
			if (vkCreateSampler(g->getVkDevice(), &_samplerCreateInfo, g->getVkAllocationCallbacks(), &_sampler) != VK_SUCCESS) {
				_oldSamplerCreateInfo = _samplerCreateInfo;
				_dirty = 0;
			}
		}
	}

	void Sampler::_release() {
		if (_sampler) {
			auto g = _graphics.get<Graphics>();
			vkDestroySampler(g->getVkDevice(), _sampler, g->getVkAllocationCallbacks());
			_sampler = nullptr;
		}
	}
}