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


	RenderTargetBlendState::RenderTargetBlendState() :
		enabled(false),
		writeMask(decltype(writeMask)::All<bool>(true)) {
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
		maxSampleCount = 0;
		simultaneousRenderTargetCount = 0;
		indexTypes.clear();
		textureFormats.clear();
	}
}