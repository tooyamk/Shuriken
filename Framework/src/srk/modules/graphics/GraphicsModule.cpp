#include "GraphicsModule.h"
#include "srk/String.h"

namespace srk::modules::graphics {
	IGraphicsModule::~IGraphicsModule() {
	}


	IObject::IObject(IGraphicsModule& graphics) :
		_graphics(&graphics) {
	}


	ProgramSource::ProgramSource() :
		language(ProgramLanguage::UNKNOWN),
		stage(ProgramStage::UNKNOWN),
		data() {
	}

	ProgramSource::ProgramSource(ProgramSource&& value) :
		language(value.language),
		stage(value.stage),
		entryPoint(std::move(value.entryPoint)),
		version(std::move(value.version)),
		data(std::move(value.data)) {
	}

	ProgramSource& ProgramSource::operator=(ProgramSource&& value) noexcept {
		language = value.language;
		stage = value.stage;
		entryPoint = std::move(value.entryPoint);
		version = std::move(value.version);
		data = std::move(value.data);

		return *this;
	}

	bool ProgramSource::isValid() const {
		return language != ProgramLanguage::UNKNOWN &&
			stage != ProgramStage::UNKNOWN &&
			data.isValid();
	}

	std::string_view ProgramSource::toHLSLShaderStage(ProgramStage stage) {
		using namespace std::literals;

		switch (stage) {
		case ProgramStage::VS:
			return "vs"sv;
		case ProgramStage::PS:
			return "ps"sv;
		case ProgramStage::GS:
			return "gs"sv;
		case ProgramStage::CS:
			return "cs"sv;
		case ProgramStage::HS:
			return "hs"sv;
		case ProgramStage::DS:
			return "ds"sv;
		default:
			return ""sv;
		}
	}

	std::string ProgramSource::toHLSLShaderModel(ProgramStage stage, const std::string_view& version) {
		std::string sm(toHLSLShaderStage(stage));

		sm.push_back('_');

		if (version.empty()) {
			sm += "5_0";
		} else {
			std::vector<std::string_view> vers;
			String::split(version, ".", [&vers](const std::string_view& data) {
				vers.emplace_back(data);
				});
			uint32_t n = vers.size();
			for (uint32_t i = 0; i < n; ++i) {
				if (i != 0) sm.push_back('_');
				sm += vers[i];
			}
		}

		return std::move(sm);
	}


	SamplerFilter::SamplerFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) :
		operation(op),
		minification(min),
		magnification(mag),
		mipmap(mipmap) {
	}

	SamplerFilter::SamplerFilter(const SamplerFilter& filter) {
		*(FeatureValueType*)this = filter.getFeatureValue();
	}

	SamplerFilter::SamplerFilter(SamplerFilter&& filter) noexcept {
		*(FeatureValueType*)this = filter.getFeatureValue();
	}


	SamplerAddress::SamplerAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) :
		u(u),
		v(v),
		w(w),
		reserved(0) {
	}

	SamplerAddress::SamplerAddress(const SamplerAddress& address) {
		*(FeatureValueType*)this = address.getFeatureValue();
	}

	SamplerAddress::SamplerAddress(SamplerAddress&& address) noexcept {
		*(FeatureValueType*)this = address.getFeatureValue();
	}


	BlendFunc::BlendFunc() : BlendFunc(BlendFactor::ONE, BlendFactor::ZERO) {
	}

	BlendFunc::BlendFunc(nullptr_t) {
	}

	BlendFunc::BlendFunc(BlendFactor src, BlendFactor dst) : BlendFunc(src, dst, src, dst) {
	}

	BlendFunc::BlendFunc(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) :
		srcColor(srcColor),
		dstColor(dstColor),
		srcAlpha(srcAlpha),
		dstAlpha(dstAlpha) {
	}

	BlendFunc::BlendFunc(const BlendFunc& func) : BlendFunc(func.srcColor, func.dstColor, func.srcAlpha, func.dstAlpha) {}


	BlendEquation::BlendEquation() : BlendEquation(BlendOp::ADD) {
	}

	BlendEquation::BlendEquation(nullptr_t) {
	}

	BlendEquation::BlendEquation(BlendOp op) : BlendEquation(op, op) {
	}

	BlendEquation::BlendEquation(BlendOp color, BlendOp alpha) :
		color(color),
		alpha(alpha) {
	}

	BlendEquation::BlendEquation(const BlendEquation& equation) {
		*(FeatureValueType*)this = equation.getFeatureValue();
	}

	BlendEquation::BlendEquation(BlendEquation&& equation) noexcept {
		*(FeatureValueType*)this = equation.getFeatureValue();
	}


	bool TextureUtils::isCompressedFormat(TextureFormat format) {
		return format >= TextureFormat::BC1_TYPELESS && format <= TextureFormat::BC7_UNORM_SRGB;
	}

	size_t TextureUtils::getBlocks(TextureFormat format, size_t pixels) {
		switch (format) {
		case TextureFormat::R8G8B8_TYPELESS:
		case TextureFormat::R8G8B8_UNORM:
		case TextureFormat::R8G8B8_UNORM_SRGB:
		case TextureFormat::R8G8B8_UINT:
		case TextureFormat::R8G8B8_SNORM:
		case TextureFormat::R8G8B8_SINT:
		case TextureFormat::R8G8B8A8_TYPELESS:
		case TextureFormat::R8G8B8A8_UNORM:
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
		case TextureFormat::R8G8B8A8_UINT:
		case TextureFormat::R8G8B8A8_SNORM:
		case TextureFormat::R8G8B8A8_SINT:
			return pixels;
		case TextureFormat::BC1_TYPELESS:
		case TextureFormat::BC1_UNORM:
		case TextureFormat::BC1_UNORM_SRGB:
		case TextureFormat::BC2_TYPELESS:
		case TextureFormat::BC2_UNORM:
		case TextureFormat::BC2_UNORM_SRGB:
		case TextureFormat::BC3_TYPELESS:
		case TextureFormat::BC3_UNORM:
		case TextureFormat::BC3_UNORM_SRGB:
		case TextureFormat::BC4_TYPELESS:
		case TextureFormat::BC4_UNORM:
		case TextureFormat::BC4_SNORM:
		case TextureFormat::BC5_TYPELESS:
		case TextureFormat::BC5_UNORM:
		case TextureFormat::BC5_SNORM:
		case TextureFormat::BC6_TYPELESS:
		case TextureFormat::BC6H_UF16:
		case TextureFormat::BC6H_SF16:
		case TextureFormat::BC7_TYPELESS:
		case TextureFormat::BC7_UNORM:
		case TextureFormat::BC7_UNORM_SRGB:
			return (pixels + 3) >> 2;
		default:
			return 0;
		}
	}

	Vec2uz TextureUtils::getBlocks(TextureFormat format, const Vec2uz& pixels) {
		switch (format) {
		case TextureFormat::R8G8B8_TYPELESS:
		case TextureFormat::R8G8B8_UNORM:
		case TextureFormat::R8G8B8_UNORM_SRGB:
		case TextureFormat::R8G8B8_UINT:
		case TextureFormat::R8G8B8_SNORM:
		case TextureFormat::R8G8B8_SINT:
		case TextureFormat::R8G8B8A8_TYPELESS:
		case TextureFormat::R8G8B8A8_UNORM:
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
		case TextureFormat::R8G8B8A8_UINT:
		case TextureFormat::R8G8B8A8_SNORM:
		case TextureFormat::R8G8B8A8_SINT:
			return pixels;
		case TextureFormat::BC1_TYPELESS:
		case TextureFormat::BC1_UNORM:
		case TextureFormat::BC1_UNORM_SRGB:
		case TextureFormat::BC2_TYPELESS:
		case TextureFormat::BC2_UNORM:
		case TextureFormat::BC2_UNORM_SRGB:
		case TextureFormat::BC3_TYPELESS:
		case TextureFormat::BC3_UNORM:
		case TextureFormat::BC3_UNORM_SRGB:
		case TextureFormat::BC4_TYPELESS:
		case TextureFormat::BC4_UNORM:
		case TextureFormat::BC4_SNORM:
		case TextureFormat::BC5_TYPELESS:
		case TextureFormat::BC5_UNORM:
		case TextureFormat::BC5_SNORM:
		case TextureFormat::BC6_TYPELESS:
		case TextureFormat::BC6H_UF16:
		case TextureFormat::BC6H_SF16:
		case TextureFormat::BC7_TYPELESS:
		case TextureFormat::BC7_UNORM:
		case TextureFormat::BC7_UNORM_SRGB:
			return (pixels + 3) >> 2;
		default:
			return Vec2uz();
		}
	}

	size_t TextureUtils::getPerBlockBytes(TextureFormat format) {
		switch (format) {
		case TextureFormat::R8G8B8_TYPELESS:
		case TextureFormat::R8G8B8_UNORM:
		case TextureFormat::R8G8B8_UNORM_SRGB:
		case TextureFormat::R8G8B8_UINT:
		case TextureFormat::R8G8B8_SNORM:
		case TextureFormat::R8G8B8_SINT:
			return 3;
		case TextureFormat::R8G8B8A8_TYPELESS:
		case TextureFormat::R8G8B8A8_UNORM:
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
		case TextureFormat::R8G8B8A8_UINT:
		case TextureFormat::R8G8B8A8_SNORM:
		case TextureFormat::R8G8B8A8_SINT:
			return 4;
		case TextureFormat::BC1_TYPELESS:
		case TextureFormat::BC1_UNORM:
		case TextureFormat::BC1_UNORM_SRGB:
		case TextureFormat::BC4_TYPELESS:
		case TextureFormat::BC4_UNORM:
		case TextureFormat::BC4_SNORM:
			return 8;
		case TextureFormat::BC2_TYPELESS:
		case TextureFormat::BC2_UNORM:
		case TextureFormat::BC2_UNORM_SRGB:
		case TextureFormat::BC3_TYPELESS:
		case TextureFormat::BC3_UNORM:
		case TextureFormat::BC3_UNORM_SRGB:
		case TextureFormat::BC5_TYPELESS:
		case TextureFormat::BC5_UNORM:
		case TextureFormat::BC5_SNORM:
		case TextureFormat::BC6_TYPELESS:
		case TextureFormat::BC6H_UF16:
		case TextureFormat::BC6H_SF16:
		case TextureFormat::BC7_TYPELESS:
		case TextureFormat::BC7_UNORM:
		case TextureFormat::BC7_UNORM_SRGB:
			return 16;
		default:
			return 0;
		}
	}

	Vec2uz TextureUtils::getPerBlockPixels(TextureFormat format) {
		switch (format) {
		case TextureFormat::R8G8B8_TYPELESS:
		case TextureFormat::R8G8B8_UNORM:
		case TextureFormat::R8G8B8_UNORM_SRGB:
		case TextureFormat::R8G8B8_UINT:
		case TextureFormat::R8G8B8_SNORM:
		case TextureFormat::R8G8B8_SINT:
		case TextureFormat::R8G8B8A8_TYPELESS:
		case TextureFormat::R8G8B8A8_UNORM:
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
		case TextureFormat::R8G8B8A8_UINT:
		case TextureFormat::R8G8B8A8_SNORM:
		case TextureFormat::R8G8B8A8_SINT:
			return Vec2uz(1, 1);
		case TextureFormat::BC1_TYPELESS:
		case TextureFormat::BC1_UNORM:
		case TextureFormat::BC1_UNORM_SRGB:
		case TextureFormat::BC2_TYPELESS:
		case TextureFormat::BC2_UNORM:
		case TextureFormat::BC2_UNORM_SRGB:
		case TextureFormat::BC3_TYPELESS:
		case TextureFormat::BC3_UNORM:
		case TextureFormat::BC3_UNORM_SRGB:
		case TextureFormat::BC4_TYPELESS:
		case TextureFormat::BC4_UNORM:
		case TextureFormat::BC4_SNORM:
		case TextureFormat::BC5_TYPELESS:
		case TextureFormat::BC5_UNORM:
		case TextureFormat::BC5_SNORM:
		case TextureFormat::BC6_TYPELESS:
		case TextureFormat::BC6H_UF16:
		case TextureFormat::BC6H_SF16:
		case TextureFormat::BC7_TYPELESS:
		case TextureFormat::BC7_UNORM:
		case TextureFormat::BC7_UNORM_SRGB:
			return Vec2uz(4, 4);
		default:
			return Vec2uz();
		}
	}


	RasterizerDescriptor::RasterizerDescriptor() :
		scissorEnabled(false),
		fillMode(FillMode::SOLID),
		cullMode(CullMode::BACK),
		frontFace(FrontFace::CW) {
	}


	RasterizerFeature::RasterizerFeature() noexcept :
		_value(0) {
	}

	RasterizerFeature::RasterizerFeature(const RasterizerFeature& other) noexcept :
		_value(other._value) {
	}

	void RasterizerFeature::set(const IRasterizerState& state) {
		_value = 1U << 31 | (state.getScissorEnabled() ? (1U << 5) : 0U) | ((uint32_t)state.getFillMode() << 3) | ((uint32_t)state.getCullMode() << 1) | (uint32_t)state.getFrontFace();
	}

	void RasterizerFeature::set(const RasterizerDescriptor& desc) noexcept {
		_value = 1U << 31 | (desc.scissorEnabled ? (1U << 5) : 0U) | ((uint32_t)desc.fillMode << 3) | ((uint32_t)desc.cullMode << 1) | (uint32_t)desc.frontFace;
	}

	bool RasterizerFeature::trySet(const RasterizerFeature& val) noexcept {
		if (_value == val._value) return false;

		_value = val._value;
		return true;
	}


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
		writeMask(VECTOR_SET_ALL, true) {
	}

	RenderTargetBlendState::RenderTargetBlendState(const RenderTargetBlendState& other) :
		equation(nullptr),
		func(nullptr),
		writeMask(nullptr) {
		*(FeatureValueType*)this = other.getFeatureValue();
	}

	RenderTargetBlendState::RenderTargetBlendState(RenderTargetBlendState&& other) noexcept :
		equation(nullptr),
		func(nullptr),
		writeMask(nullptr) {
		*(FeatureValueType*)this = other.getFeatureValue();
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
		maxSamplerAnisotropy = 0;
		indexTypes.clear();
		textureFormats.clear();
	}
}