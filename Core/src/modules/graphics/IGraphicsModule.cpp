#include "IGraphicsModule.h"
#include "base/String.h"

namespace aurora::modules::graphics {
	IGraphicsModule::~IGraphicsModule() {
	}


	IObject::IObject(IGraphicsModule& graphics) :
		_graphics(&graphics) {
	}


	SamplerFilter::SamplerFilter() :
		operation(SamplerFilterOperation::NORMAL),
		minification(SamplerFilterMode::LINEAR),
		magnification(SamplerFilterMode::LINEAR),
		mipmap(SamplerFilterMode::LINEAR) {
	}


	SamplerAddress::SamplerAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) :
		u(u),
		v(v),
		w(w) {
	}
}