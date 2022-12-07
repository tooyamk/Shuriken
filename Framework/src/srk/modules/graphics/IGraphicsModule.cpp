#include "IGraphicsModule.h"
#include "srk/String.h"

namespace srk::modules::graphics {
	IGraphicsModule::~IGraphicsModule() {
	}


	IObject::IObject(IGraphicsModule& graphics) :
		_graphics(&graphics) {
	}


	SamplerFilter::SamplerFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) :
		operation(op),
		minification(min),
		magnification(mag),
		mipmap(mipmap) {
	}

	SamplerFilter::SamplerFilter(const SamplerFilter& filter) : SamplerFilter(filter.operation, filter.minification, filter.magnification, filter.mipmap) {}


	SamplerAddress::SamplerAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) :
		u(u),
		v(v),
		w(w) {
	}


	BlendFunc::BlendFunc() : BlendFunc(BlendFactor::ONE, BlendFactor::ZERO) {}

	BlendFunc::BlendFunc(BlendFactor src, BlendFactor dst) : BlendFunc(src, dst, src, dst) {}

	BlendFunc::BlendFunc(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) :
		srcColor(srcColor),
		dstColor(dstColor),
		srcAlpha(srcAlpha),
		dstAlpha(dstAlpha) {
	}

	BlendFunc::BlendFunc(const BlendFunc& func) : BlendFunc(func.srcColor, func.dstColor, func.srcAlpha, func.dstAlpha) {}


	BlendEquation::BlendEquation() : BlendEquation(BlendOp::ADD) {}

	BlendEquation::BlendEquation(BlendOp op) : BlendEquation(op, op) {}

	BlendEquation::BlendEquation(BlendOp color, BlendOp alpha) :
		color(color),
		alpha(alpha) {
	}

	BlendEquation::BlendEquation(const BlendEquation& equation) : BlendEquation(equation.color, equation.alpha) {}


	DepthStencilFeature::DepthStencilFeature() noexcept :
		_low(0),
		_high(0) {
	}

	DepthStencilFeature::DepthStencilFeature(const DepthStencilFeature& other) noexcept :
		_low(other._low),
		_high(other._high) {
	}

	void DepthStencilFeature::set(const DepthState& depth, const StencilState& stencil) {
		_setValid();
		if (depth.enabled) {
			_low |= 0x1ULL << 54;
			if (depth.writeable) _low |= 0x1ULL << 53;
			_low |= (uint64_t)depth.func << 49;
		}
		_set(stencil);
	}

	void DepthStencilFeature::set(const StencilState& stencil) {
		_setValid();
		_set(stencil);
	}

	bool DepthStencilFeature::trySet(const DepthStencilFeature& val) noexcept {
		if (_low == val._low && _high == val._high) return false;

		_low = val._low;
		_high = val._high;
		return true;
	}

	void DepthStencilFeature::_setValid() noexcept {
		_high = 0x1ULL << 63;
		_low = 0;
	}

	void DepthStencilFeature::_set(const StencilState& stencil) {
		if (stencil.enabled) {
			_low |= 0x1ULL << 48;
			_low |= (uint64_t)(*((uint16_t*)&stencil.face.front.mask)) << 32;
			_low |= (uint64_t)stencil.face.front.op.fail << 28;
			_low |= (uint64_t)stencil.face.front.op.depthFail << 24;
			_low |= (uint64_t)stencil.face.front.op.pass << 20;
			_low |= (uint64_t)stencil.face.front.func << 16;
			_low |= (uint64_t)stencil.face.back.op.fail << 12;
			_low |= (uint64_t)stencil.face.back.op.depthFail << 8;
			_low |= (uint64_t)stencil.face.back.op.pass << 4;
			_low |= (uint64_t)stencil.face.back.func << 0;
			_high |= (uint64_t)stencil.face.front.ref;
			_high |= (uint64_t)stencil.face.back.ref << 8;
		}
	}


	RenderTargetBlendState::RenderTargetBlendState() :
		enabled(false),
		equation(BlendOp::ADD),
		func(BlendFactor::ONE, BlendFactor::ZERO),
		reserved(0),
		writeMask(VECTOR_SET_ALL, true) {
	}


	DepthState::DepthState() :
		enabled(true),
		writeable(true),
		func(ComparisonFunc::LESS),
		reserved(0) {
	}


	StencilFaceState::StencilFaceState() :
		func(ComparisonFunc::ALWAYS),
		op({ StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP }),
		ref(0),
		reserved(0) {
		mask.read = std::numeric_limits<decltype(mask.read)>::max();
		mask.write = std::numeric_limits<decltype(mask.read)>::max();
	}


	GraphicsDeviceFeatures::GraphicsDeviceFeatures() {
		reset();
	}

	void GraphicsDeviceFeatures::reset() {
		sampler = false;
		nativeTextureView = false;
		nativeRenderView = false;
		pixelBuffer = false;
		constantBuffer = false;
		textureMap = false;
		persistentMap = false;
		independentBlend = false;
		stencilIndependentRef = false;
		stencilIndependentMask = false;
		vertexDim3Bit8 = false;
		vertexDim3Bit16 = false;
		maxSampleCount = 0;
		simultaneousRenderTargetCount = 0;
		indexTypes.clear();
		textureFormats.clear();
	}
}