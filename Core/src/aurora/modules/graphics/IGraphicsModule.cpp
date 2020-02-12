#include "IGraphicsModule.h"
#include "aurora/String.h"

namespace aurora::modules::graphics {
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
		op({ BlendOp::ADD, BlendOp::ADD } ),
		writeMask(true) {
	}
}