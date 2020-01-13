#pragma once

#include "modules/IModule.h"
#include "math/Box.h"

namespace aurora::modules::graphics {
	class IGraphicsModule;
	class GraphicsAdapter;
	class ProgramSource;
	class ShaderParameterFactory;
	class VertexBufferFactory;


	class AE_DLL IObject : public Ref {
	public:
		virtual ~IObject() {}

		inline IGraphicsModule* AE_CALL getGraphics() const {
			return _graphics.get();
		}

	protected:
		IObject(IGraphicsModule& graphics);

		RefPtr<IGraphicsModule> _graphics;
	};


	enum class Usage : uint8_t {
		NONE = 0,//create and map
		MAP_READ = 1,//create and map
		MAP_WRITE = 1 << 1,//create and map
		UPDATE = 1 << 2,//create

		PERSISTENT_MAP = 1 << 3,//create

		MAP_SWAP = 1 << 4,//map
		MAP_FORCE_SWAP = (1 << 5) | MAP_SWAP,//map

		DISCARD = 1 << 6,//map return

		MAP_READ_WRITE = MAP_READ | MAP_WRITE,
		MAP_WRITE_UPDATE = MAP_WRITE | UPDATE
	};
	AE_DEFINE_ENUM_BIT_OPERATIION(Usage);


	class AE_DLL IBuffer : public IObject {
	public:
		IBuffer(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBuffer() {}

		virtual const void* AE_CALL getNativeBuffer() const = 0;
		virtual bool AE_CALL create(uint32_t size, Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) = 0;
		virtual uint32_t AE_CALL getSize() const = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap() = 0;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) = 0;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) = 0;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) = 0;
		//virtual void AE_CALL flush() = 0;
		virtual bool AE_CALL isSyncing() const = 0;
	};


	enum class VertexSize : uint8_t {
		UNKNOWN,
		ONE,
		TWO,
		THREE,
		FOUR
	};


	enum class VertexType : uint8_t {
		UNKNOWN,
		I8,
		UI8,
		I16,
		UI16,
		I32,
		UI32,
		F32
	};


	class AE_DLL IVertexBuffer : public IBuffer {
	public:
		IVertexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IVertexBuffer() {}

		virtual void AE_CALL getFormat(VertexSize* size, VertexType* type) const = 0;
		virtual void AE_CALL setFormat(VertexSize size, VertexType type) = 0;
	};


	enum class IndexType : uint8_t {
		UNKNOWN,
		UI8,
		UI16,
		UI32
	};


	class AE_DLL IIndexBuffer : public IBuffer {
	public:
		IIndexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IIndexBuffer() {}

		virtual IndexType AE_CALL getFormat() const = 0;
		virtual void AE_CALL setFormat(IndexType type) = 0;
	};


	class AE_DLL IConstantBuffer : public IBuffer {
	public:
		virtual ~IConstantBuffer() {}

	protected:
		IConstantBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
	};


	class AE_DLL IPixelBuffer : public IBuffer {
	public:
		virtual ~IPixelBuffer() {}

	protected:
		IPixelBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
	};


	enum class SamplerFilterMode : uint8_t {
		POINT,
		LINEAR,
		ANISOTROPIC
	};


	enum class SamplerFilterOperation : uint8_t {
		NORMAL,
		COMPARISON,
		MINIMUM,
		MAXIMUM
	};


	enum class SamplerComparisonFunc : uint8_t {
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};


	enum class SamplerAddressMode : uint8_t {
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE
	};


	struct AE_DLL SamplerFilter {
		SamplerFilter(SamplerFilterOperation op = SamplerFilterOperation::NORMAL, SamplerFilterMode min = SamplerFilterMode::LINEAR, SamplerFilterMode mag = SamplerFilterMode::LINEAR, SamplerFilterMode mipmap = SamplerFilterMode::LINEAR);
		SamplerFilter(const SamplerFilter& filter);

		inline bool AE_CALL operator==(const SamplerFilter& val) const {
			return memEqual<sizeof(SamplerFilter)>(this, &val);
		}

		inline bool AE_CALL operator!=(const SamplerFilter& val) const {
			return !memEqual<sizeof(SamplerFilter)>(this, &val);
		}

		inline void AE_CALL operator=(const SamplerFilter& val) {
			memcpy(this, &val, sizeof(SamplerFilter));
		}

		SamplerFilterOperation operation;
		SamplerFilterMode minification;
		SamplerFilterMode magnification;
		SamplerFilterMode mipmap;
	};


	struct AE_DLL SamplerAddress {
		SamplerAddress(SamplerAddressMode u = SamplerAddressMode::WRAP, SamplerAddressMode v = SamplerAddressMode::WRAP, SamplerAddressMode w = SamplerAddressMode::WRAP);


		inline bool AE_CALL operator==(const SamplerAddress& val) const {
			return memEqual<sizeof(SamplerAddress)>(this, &val);
		}

		inline bool AE_CALL operator!=(const SamplerAddress& val) const {
			return !memEqual<sizeof(SamplerAddress)>(this, &val);
		}

		inline void AE_CALL operator=(const SamplerAddress& val) {
			memcpy(this, &val, sizeof(SamplerAddress));
		}

		SamplerAddressMode u;
		SamplerAddressMode v;
		SamplerAddressMode w;
	};


	class AE_DLL ISampler : public IObject {
	public:
		ISampler(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ISampler() {}

		inline void AE_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) {
			setFilter(SamplerFilter(op, min, mag, mipmap));
		}
		virtual void AE_CALL setFilter(const SamplerFilter& filter) = 0;

		virtual void AE_CALL setComparisonFunc(SamplerComparisonFunc func) = 0;

		inline void AE_CALL setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) {
			setAddress(SamplerAddress(u, v, w));
		}
		virtual void AE_CALL setAddress(const SamplerAddress& address) = 0;

		virtual void AE_CALL setMipLOD(f32 min, f32 max) = 0;
		virtual void AE_CALL setMipLODBias(f32 bias) = 0;
		virtual void AE_CALL setMaxAnisotropy (uint32_t max) = 0;

		virtual void AE_CALL setBorderColor(const Vec4f32& color) = 0;
	};


	enum class TextureType : uint8_t {
		TEX1D,
		TEX2D,
		TEX3D
	};


	enum class TextureFormat : uint8_t {
		UNKNOWN,
		R8G8B8,
		R8G8B8A8
	};


	class AE_DLL ITextureViewBase : public IObject {
	public:
		ITextureViewBase(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ITextureViewBase() {}

		//virtual TextureType AE_CALL getType() const = 0;
		virtual const void* AE_CALL getNativeView() const = 0;
		virtual uint32_t AE_CALL getArraySize() const = 0;
		virtual uint32_t AE_CALL getMipLevels() const = 0;
	};


	class AE_DLL ITextureResource : public ITextureViewBase {
	public:
		ITextureResource(IGraphicsModule& graphics) : ITextureViewBase(graphics) {}
		virtual ~ITextureResource() {}

		virtual TextureType AE_CALL getType() const = 0;
		virtual const void* AE_CALL getNativeResource() const = 0;
		virtual uint16_t AE_CALL getPerPixelByteSize() const = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap(uint32_t arraySlice, uint32_t mipSlice) = 0;
		virtual uint32_t AE_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) = 0;
		virtual uint32_t AE_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) = 0;
	};


	class AE_DLL ITexture1DResource : public ITextureResource {
	public:
		ITexture1DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture1DResource() {}

		virtual bool AE_CALL create(uint32_t width, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const void* data) = 0;
		virtual bool AE_CALL copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const IPixelBuffer* pixelBuffer) = 0;
	};


	class AE_DLL ITexture2DResource : public ITextureResource {
	public:
		ITexture2DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture2DResource() {}

		virtual bool AE_CALL create(const Vec2ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const void* data) = 0;
		virtual bool AE_CALL copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const IPixelBuffer* pixelBuffer) = 0;
	};


	class AE_DLL ITexture3DResource : public ITextureResource {
	public:
		ITexture3DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture3DResource() {}

		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) = 0;
		virtual bool AE_CALL copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) = 0;
	};


	class AE_DLL ITextureView : public ITextureViewBase {
	public:
		ITextureView(IGraphicsModule& graphics) : ITextureViewBase(graphics) {}
		virtual ~ITextureView() {}

		//virtual TextureType AE_CALL getType() const = 0;
		virtual ITextureResource* AE_CALL getResource() const = 0;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) = 0;
	};


	enum class BlendFactor : uint8_t {
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		SRC_ALPHA_SATURATE,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA
	};


	enum class BlendOp : uint8_t {
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX
	};


	enum class BlendLogicOp : uint8_t {
		CLEAR,
		SET,
		COPY,
		COPY_INVERTED,
		NOOP,
		INVERT,
		AND,
		NAND,
		OR,
		NOR,
		XOR,
		EQUIV,
		AND_REVERSE,
		AND_INVERTED,
		OR_REVERSE,
		OR_INVERTED
	};


	struct AE_DLL BlendFunc {
		BlendFunc();
		BlendFunc(BlendFactor src, BlendFactor dst);
		BlendFunc(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);
		BlendFunc(const BlendFunc& func);

		inline bool AE_CALL operator==(const BlendFunc& val) const {
			return memEqual<sizeof(BlendFunc)>(this, &val);
		}

		inline bool AE_CALL operator!=(const BlendFunc& val) const {
			return !memEqual<sizeof(BlendFunc)>(this, &val);
		}

		inline void AE_CALL operator=(const BlendFunc& val) {
			memcpy(this, &val, sizeof(BlendFunc));
		}
		
		inline BlendFunc& AE_CALL set(BlendFactor src, BlendFactor dst) {
			return set(src, dst, src, dst);
		}

		inline BlendFunc& AE_CALL set(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) {
			this->srcColor = srcColor;
			this->dstColor = dstColor;
			this->srcAlpha = srcAlpha;
			this->dstAlpha = dstAlpha;
			
			return *this;
		}

		BlendFactor srcColor;
		BlendFactor dstColor;
		BlendFactor srcAlpha;
		BlendFactor dstAlpha;
	};


	struct AE_DLL BlendEquation {
		BlendEquation();
		BlendEquation(BlendOp op);
		BlendEquation(BlendOp color, BlendOp alpha);
		BlendEquation(const BlendEquation& op);

		inline bool AE_CALL operator==(const BlendEquation& val) const {
			return memEqual<sizeof(BlendEquation)>(this, &val);
		}

		inline bool AE_CALL operator!=(const BlendEquation& val) const {
			return !memEqual<sizeof(BlendEquation)>(this, &val);
		}

		inline void AE_CALL operator=(const BlendEquation& val) {
			memcpy(this, &val, sizeof(BlendEquation));
		}

		inline BlendEquation& AE_CALL set(BlendOp op) {
			return set(op, op);
		}

		inline BlendEquation& AE_CALL set(BlendOp color, BlendOp alpha) {
			this->color = color;
			this->alpha = alpha;

			return *this;
		}

		BlendOp color;
		BlendOp alpha;
	};


	class AE_DLL RenderTargetBlendState {
	public:
		RenderTargetBlendState();

		bool enabled;
		BlendFunc func;
		struct {
			BlendOp color;
			BlendOp alpha;
		} op;
		Vec4<bool> writeMask;
	};


	class AE_DLL IBlendState : public IObject {
	public:
		IBlendState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBlendState() {}

		virtual bool AE_CALL isIndependentBlendEnabled() const = 0;
		virtual void AE_CALL setIndependentBlendEnabled(bool enalbed) = 0;

		virtual const RenderTargetBlendState& AE_CALL getRenderTargetState(uint8_t index) const = 0;
		virtual void AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) = 0;
	};


	enum class ProgramStage : uint8_t {
		UNKNOWN,
		VS,//VertexShader
		PS,//PixelShader
		GS,//GeomtryShader
		CS,//ComputeShader
		HS,//HullShader
		DS //DomainShader
	};


	class AE_DLL IProgram : public IObject {
	public:
		IProgram(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IProgram() {}

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) = 0;
	};


	enum class FillMode : uint8_t {
		WIREFRAME,
		SOLID
	};


	enum class CullMode : uint8_t {
		NONE,
		FRONT,
		BACK
	};


	enum class FrontFace : uint8_t {
		CW,
		CCW
	};


	class AE_DLL IRasterizerState : public IObject {
	public:
		IRasterizerState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRasterizerState() {}

		virtual FillMode AE_CALL getFillMode() const = 0;
		virtual void AE_CALL setFillMode(FillMode fill) = 0;

		virtual CullMode AE_CALL getCullMode() const = 0;
		virtual void AE_CALL setCullMode(CullMode cull) = 0;

		virtual FrontFace AE_CALL getFrontFace() const = 0;
		virtual void AE_CALL setFrontFace(FrontFace front) = 0;
	};


	enum class ClearFlag : uint8_t {
		NONE,
		COLOR,
		DEPTH,
		STENCIL
	};
	AE_DEFINE_ENUM_BIT_OPERATIION(ClearFlag);


	struct AE_DLL GraphicsDeviceFeatures {
		bool supportSampler;
		bool supportTextureView;
		bool supportPixelBuffer;
		bool supportConstantBuffer;
		bool supportPersistentMap;
		bool supportIndependentBlend;
	};


	class AE_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ModuleType AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual const std::string& AE_CALL getVersion() const = 0;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const = 0;
		virtual IBlendState* AE_CALL createBlendState() = 0;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() = 0;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() = 0;
		virtual IPixelBuffer* AE_CALL createPixelBuffer() = 0;
		virtual IProgram* AE_CALL createProgram() = 0;
		virtual IRasterizerState* AE_CALL createRasterizerState() = 0;
		virtual ISampler* AE_CALL createSampler() = 0;
		virtual ITexture1DResource* AE_CALL createTexture1DResource() = 0;//unrealized wingl
		virtual ITexture2DResource* AE_CALL createTexture2DResource() = 0;
		virtual ITexture3DResource* AE_CALL createTexture3DResource() = 0;//unrealized wingl
		virtual ITextureView* AE_CALL createTextureView() = 0;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() = 0;

		virtual void AE_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) = 0;//unrealized all sampleMask
		virtual void AE_CALL setRasterizerState(IRasterizerState* state) = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, IProgram* program, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		//unrealized wind3d11 depth stencil
		virtual void AE_CALL clear(ClearFlag flag, const Vec4f32& color, f32 depth, size_t stencil) = 0;
	};
}