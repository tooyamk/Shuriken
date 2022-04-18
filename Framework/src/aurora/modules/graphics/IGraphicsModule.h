#pragma once

#include "aurora/modules/IModule.h"
#include "aurora/ByteArray.h"
#include "aurora/math/Box.h"
#include <functional>
#include <vector>

namespace aurora {
	enum class ProgramStage : uint8_t;
	class IShaderParameterGetter;
	class IVertexBufferGetter;
	class ProgramSource;
	class ShaderDefine;
}

namespace aurora::events {
	template<typename EvtType>
	class IEventDispatcher;
}

namespace aurora::modules::graphics {
	class IGraphicsModule;
	class GraphicsAdapter;

	using SampleCount = uint8_t;

	class AE_FW_DLL IObject : public Ref {
	public:
		virtual ~IObject() {}

		inline const IntrusivePtr<IGraphicsModule>& AE_CALL getGraphics() const {
			return _graphics;
		}

		inline IntrusivePtr<IGraphicsModule> AE_CALL getGraphics() {
			return _graphics;
		}

	protected:
		IObject(IGraphicsModule& graphics);

		IntrusivePtr<IGraphicsModule> _graphics;
	};


	enum class Usage : uint16_t {
		NONE = 0,//create and map
		MAP_READ = 1,//create and map
		MAP_WRITE = 1 << 1,//create and map
		UPDATE = 1 << 2,//create

		PERSISTENT_MAP = 1 << 3,//create
		IGNORE_UNSUPPORTED = 1 << 4,//create
		RENDERABLE = 1 << 5,//create

		MAP_SWAP = 1 << 6,//map
		MAP_FORCE_SWAP = (1 << 7) | MAP_SWAP,//map

		DISCARD = 1 << 8,//map return

		MAP_READ_WRITE = MAP_READ | MAP_WRITE,
		MAP_WRITE_UPDATE = MAP_WRITE | UPDATE
	};


	class AE_FW_DLL IBuffer : public IObject {
	public:
		IBuffer(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBuffer() {}

		virtual bool AE_CALL isCreated() const = 0;
		virtual const void* AE_CALL getNative() const = 0;
		virtual bool AE_CALL create(size_t size, Usage bufferUsage, const void* data = nullptr, size_t dataSize = 0) = 0;
		virtual size_t AE_CALL getSize() const = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap() = 0;
		virtual size_t AE_CALL read(size_t offset, void* dst, size_t dstLen) = 0;
		virtual size_t AE_CALL write(size_t offset, const void* data, size_t length) = 0;
		virtual size_t AE_CALL update(size_t offset, const void* data, size_t length) = 0;
		//virtual void AE_CALL flush() = 0;
		virtual bool AE_CALL isSyncing() const = 0;
		virtual void AE_CALL destroy() = 0;
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


	struct AE_FW_DLL VertexFormat {
		VertexFormat() :
			size(VertexSize::UNKNOWN),
			type(VertexType::UNKNOWN) {
		}

		VertexFormat(VertexSize size, VertexType type) :
			size(size),
			type(type) {
		}

		inline bool AE_CALL operator==(const VertexFormat& val) const {
			return featureValue == val.featureValue;
		}

		inline bool AE_CALL operator!=(const VertexFormat& val) const {
			return featureValue != val.featureValue;
		}

		inline void AE_CALL operator=(const VertexFormat& val) {
			featureValue = val.featureValue;
		}

		template<uint32_t Size>
		inline void AE_CALL set() {
			if constexpr (Size <= (decltype(Size))VertexSize::FOUR) {
				size = (VertexSize)Size;
			} else {
				size = VertexSize::UNKNOWN;
			}
		}

		template<typename Type>
		inline void AE_CALL set() {
			if constexpr (std::same_as<Type, int8_t>) {
				type = VertexType::I8;
			} else if constexpr (std::same_as<Type, uint8_t>) {
				type = VertexType::UI8;
			} else if constexpr (std::same_as<Type, int16_t>) {
				type = VertexType::I16;
			} else if constexpr (std::same_as<Type, uint16_t>) {
				type = VertexType::UI16;
			} else if constexpr (std::same_as<Type, int32_t>) {
				type = VertexType::I32;
			} else if constexpr (std::same_as<Type, uint32_t>) {
				type = VertexType::UI32;
			} else if constexpr (std::same_as<Type, float32_t>) {
				type = VertexType::F32;
			} else {
				type = VertexType::UNKNOWN;
			}
		}

		template<uint32_t Size, typename Type>
		inline void AE_CALL set() {
			set<Size>();
			set<Type>();
		}

		union {
			uint16_t featureValue;

			struct {
				VertexSize size;
				VertexType type;
			};
		};
	};


	class AE_FW_DLL IVertexBuffer : public IBuffer {
	public:
		IVertexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IVertexBuffer() {}

		virtual const VertexFormat& AE_CALL getFormat() const = 0;
		virtual void AE_CALL setFormat(const VertexFormat& format) = 0;
	};


	enum class IndexType : uint8_t {
		UNKNOWN,
		UI8,
		UI16,
		UI32
	};


	class AE_FW_DLL IIndexBuffer : public IBuffer {
	public:
		IIndexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IIndexBuffer() {}

		virtual IndexType AE_CALL getFormat() const = 0;
		virtual void AE_CALL setFormat(IndexType type) = 0;

		template<typename T>
		inline void AE_CALL setFormat() {
			if constexpr (std::same_as<T, uint8_t>) {
				setFormat(IndexType::UI8);
			} else if constexpr (std::same_as<T, uint16_t>) {
				setFormat(IndexType::UI16);
			} else if constexpr (std::same_as<T, uint32_t>) {
				setFormat(IndexType::UI32);
			} else {
				setFormat(IndexType::UNKNOWN);
			}
		}
	};


	class AE_FW_DLL IConstantBuffer : public IBuffer {
	public:
		virtual ~IConstantBuffer() {}

	protected:
		IConstantBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
	};


	class AE_FW_DLL IPixelBuffer : public IBuffer {
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


	enum class ComparisonFunc : uint8_t {
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


	struct AE_FW_DLL SamplerFilter {
		SamplerFilter(SamplerFilterOperation op = SamplerFilterOperation::NORMAL, SamplerFilterMode min = SamplerFilterMode::LINEAR, SamplerFilterMode mag = SamplerFilterMode::LINEAR, SamplerFilterMode mipmap = SamplerFilterMode::LINEAR);
		SamplerFilter(const SamplerFilter& filter);

		inline bool AE_CALL operator==(const SamplerFilter& val) const {
			return featureValue == val.featureValue;
		}

		inline bool AE_CALL operator!=(const SamplerFilter& val) const {
			return featureValue != val.featureValue;
		}

		inline void AE_CALL operator=(const SamplerFilter& val) {
			featureValue = val.featureValue;
		}

		union {
			uint32_t featureValue;

			struct {
				SamplerFilterOperation operation;
				SamplerFilterMode minification;
				SamplerFilterMode magnification;
				SamplerFilterMode mipmap;
			};
		};

		
	};


	struct AE_FW_DLL SamplerAddress {
		SamplerAddress(SamplerAddressMode u = SamplerAddressMode::WRAP, SamplerAddressMode v = SamplerAddressMode::WRAP, SamplerAddressMode w = SamplerAddressMode::WRAP);


		inline bool AE_CALL operator==(const SamplerAddress& val) const {
			return !memcmp(this, &val, sizeof(SamplerAddress));
		}

		inline bool AE_CALL operator!=(const SamplerAddress& val) const {
			return memcmp(this, &val, sizeof(SamplerAddress));
		}

		inline void AE_CALL operator=(const SamplerAddress& val) {
			memcpy(this, &val, sizeof(SamplerAddress));
		}

		SamplerAddressMode u;
		SamplerAddressMode v;
		SamplerAddressMode w;
	};


	class AE_FW_DLL ISampler : public IObject {
	public:
		ISampler(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ISampler() {}

		virtual const void* AE_CALL getNative() const = 0;

		inline void AE_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) {
			setFilter(SamplerFilter(op, min, mag, mipmap));
		}
		virtual void AE_CALL setFilter(const SamplerFilter& filter) = 0;

		virtual void AE_CALL setComparisonFunc(ComparisonFunc func) = 0;

		inline void AE_CALL setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) {
			setAddress(SamplerAddress(u, v, w));
		}
		virtual void AE_CALL setAddress(const SamplerAddress& address) = 0;

		virtual void AE_CALL setMipLOD(float32_t min, float32_t max) = 0;
		virtual void AE_CALL setMipLODBias(float32_t bias) = 0;
		virtual void AE_CALL setMaxAnisotropy(uint32_t max) = 0;

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


	class AE_FW_DLL ITextureResource : public IObject {
	public:
		ITextureResource(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ITextureResource() {}

		virtual TextureType AE_CALL getType() const = 0;
		virtual bool AE_CALL isCreated() const = 0;
		virtual const void* AE_CALL getNative() const = 0;
		virtual SampleCount AE_CALL getSampleCount() const = 0;
		virtual uint16_t AE_CALL getPerPixelByteSize() const = 0;
		virtual TextureFormat AE_CALL getFormat() const = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual const Vec3ui32& AE_CALL getSize() const = 0;
		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage resUsage, const void* const* data = nullptr) = 0;
		virtual Usage AE_CALL map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap(uint32_t arraySlice, uint32_t mipSlice) = 0;
		virtual uint32_t AE_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) = 0;
		virtual uint32_t AE_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) = 0;
		virtual bool AE_CALL copyFrom(const Vec3ui32& dstPos, uint32_t dstArraySlice, uint32_t dstMipSlice, const ITextureResource* src, uint32_t srcArraySlice, uint32_t srcMipSlice, const Box3ui32& srcRange) = 0;
		virtual bool AE_CALL copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) = 0;
		virtual bool AE_CALL copyTo(uint32_t mipSlice, const IPixelBuffer* pixelBuffer) = 0;
		virtual void AE_CALL destroy() = 0;
	};


	class AE_FW_DLL ITexture1DResource : public ITextureResource {
	public:
		ITexture1DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture1DResource() {}

		virtual bool AE_CALL create(uint32_t width, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const void* data) = 0;

		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage resUsage, const void* const* data = nullptr) override {
			return create(size[0], arraySize, mipLevels, format, resUsage, data);
		}
	};


	class AE_FW_DLL ITexture2DResource : public ITextureResource {
	public:
		ITexture2DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture2DResource() {}

		virtual bool AE_CALL create(const Vec2ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box2ui32& range, const void* data) = 0;

		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage resUsage, const void* const* data = nullptr) override {
			return create((const Vec2ui32&)size, arraySize, mipLevels, sampleCount, format, resUsage, data);
		}
	};


	class AE_FW_DLL ITexture3DResource : public ITextureResource {
	public:
		ITexture3DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture3DResource() {}

		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) = 0;

		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage resUsage, const void* const* data = nullptr) override {
			return create(size, arraySize, mipLevels, format, resUsage, data);
		}
	};


	class AE_FW_DLL ITextureView : public IObject {
	public:
		ITextureView(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ITextureView() {}

		//virtual TextureType AE_CALL getType() const = 0;
		virtual bool AE_CALL isCreated() const = 0;
		virtual IntrusivePtr<ITextureResource> AE_CALL getResource() const = 0;
		virtual const void* AE_CALL getNative() const = 0;
		virtual uint32_t AE_CALL getArraySize() const = 0;
		virtual uint32_t AE_CALL getMipLevels() const = 0;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) = 0;
		virtual void AE_CALL destroy() = 0;
	};


	class AE_FW_DLL IRenderView : public IObject {
	public:
		IRenderView(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRenderView() {}

		virtual bool AE_CALL isCreated() const = 0;
		virtual IntrusivePtr<ITextureResource> AE_CALL getResource() const = 0;
		virtual const void* AE_CALL getNative() const = 0;
		virtual uint32_t AE_CALL getArraySize() const = 0;
		virtual uint32_t AE_CALL getMipSlice() const = 0;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) = 0;
		virtual void AE_CALL destroy() = 0;
	};


	enum class DepthStencilFormat : uint8_t {
		UNKNOWN,
		D16,
		D24,
		D24S8,
		D32,
		D32S8
	};


	class AE_FW_DLL IDepthStencil : public IObject {
	public:
		IDepthStencil(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IDepthStencil() {}

		virtual const void* AE_CALL getNative() const = 0;
		virtual SampleCount AE_CALL getSampleCount() const = 0;
		virtual const Vec2ui32& AE_CALL getSize() const = 0;
		virtual bool AE_CALL create(const Vec2ui32& size, DepthStencilFormat format, SampleCount sampleCount) = 0;
		virtual void AE_CALL destroy() = 0;
	};


	class AE_FW_DLL IRenderTarget : public IObject {
	public:
		IRenderTarget(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRenderTarget() {}

		virtual const void* AE_CALL getNative() const = 0;

		virtual Vec2ui32 AE_CALL getSize() const = 0;
		virtual IntrusivePtr<IRenderView> AE_CALL getRenderView(uint8_t index) const = 0;
		virtual bool AE_CALL setRenderView(uint8_t index, IRenderView* view) = 0;
		virtual void AE_CALL eraseRenderViews(uint8_t begin, uint8_t size) = 0;

		virtual IntrusivePtr<IDepthStencil> AE_CALL getDepthStencil() const = 0;
		virtual void AE_CALL setDepthStencil(IDepthStencil* ds) = 0;
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


	struct AE_FW_DLL BlendFunc {
		BlendFunc();
		BlendFunc(BlendFactor src, BlendFactor dst);
		BlendFunc(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);
		BlendFunc(const BlendFunc& func);

		inline bool AE_CALL operator==(const BlendFunc& val) const {
			return featureValue == val.featureValue;
		}

		inline bool AE_CALL operator!=(const BlendFunc& val) const {
			return featureValue != val.featureValue;
		}

		inline void AE_CALL operator=(const BlendFunc& val) {
			featureValue = val.featureValue;
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

		union {
			uint32_t featureValue;

			struct {
				BlendFactor srcColor;
				BlendFactor dstColor;
				BlendFactor srcAlpha;
				BlendFactor dstAlpha;
			};
		};
	};


	struct AE_FW_DLL BlendEquation {
		BlendEquation();
		BlendEquation(BlendOp op);
		BlendEquation(BlendOp color, BlendOp alpha);
		BlendEquation(const BlendEquation& op);

		inline bool AE_CALL operator==(const BlendEquation& val) const {
			return featureValue == val.featureValue;
		}

		inline bool AE_CALL operator!=(const BlendEquation& val) const {
			return featureValue != val.featureValue;
		}

		inline void AE_CALL operator=(const BlendEquation& val) {
			featureValue = val.featureValue;
		}

		inline BlendEquation& AE_CALL set(BlendOp op) {
			return set(op, op);
		}

		inline BlendEquation& AE_CALL set(BlendOp color, BlendOp alpha) {
			this->color = color;
			this->alpha = alpha;

			return *this;
		}

		union {
			uint16_t featureValue;

			struct {
				BlendOp color;
				BlendOp alpha;
			};
		};
	};


	class AE_FW_DLL RenderTargetBlendState {
	public:
		RenderTargetBlendState();

		bool enabled;
		BlendEquation equation;
		Vec4<bool> writeMask;
		BlendFunc func;

		inline bool AE_CALL operator==(const RenderTargetBlendState& val) const {
			return enabled == val.enabled && func == val.func && equation == val.equation && writeMask == val.writeMask;
		}

		inline bool AE_CALL operator!=(const RenderTargetBlendState& val) const {
			return enabled != val.enabled || func != val.func || equation != val.equation || writeMask != val.writeMask;
		}
	};


	class AE_FW_DLL IBlendState : public IObject {
	public:
		IBlendState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBlendState() {}

		virtual const void* AE_CALL getNative() const = 0;
		virtual uint8_t AE_CALL getCount() const = 0;
		virtual void AE_CALL setCount(uint8_t count) = 0;

		virtual const RenderTargetBlendState* AE_CALL getRenderTargetState(uint8_t index) const = 0;
		virtual bool AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) = 0;
	};


	class AE_FW_DLL ProgramInfo {
	public:
		class AE_FW_DLL Vertex {
		public:
			std::string name;
			VertexFormat format;
			bool instanced = false;
		};


		std::vector<Vertex> vertices;

		inline void AE_CALL clear() {
			vertices.clear();
		}
	};


	class AE_FW_DLL IProgram : public IObject {
	public:
		struct AE_FW_DLL InputDescription {
			bool instanced = false;
		};

		using IncludeHandler = std::function<ByteArray(const IProgram&, ProgramStage, const std::string_view&)>;
		using InputHandler = std::function<InputDescription(const IProgram&, const std::string_view&)>;

		IProgram(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IProgram() {}

		virtual const void* AE_CALL getNative() const = 0;
		virtual bool AE_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) = 0;
		virtual const ProgramInfo& getInfo() const = 0;
		virtual void AE_CALL destroy() = 0;
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


	class AE_FW_DLL IRasterizerState : public IObject {
	public:
		IRasterizerState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRasterizerState() {}

		virtual const void* AE_CALL getNative() const = 0;

		virtual FillMode AE_CALL getFillMode() const = 0;
		virtual void AE_CALL setFillMode(FillMode fill) = 0;

		virtual CullMode AE_CALL getCullMode() const = 0;
		virtual void AE_CALL setCullMode(CullMode cull) = 0;

		virtual FrontFace AE_CALL getFrontFace() const = 0;
		virtual void AE_CALL setFrontFace(FrontFace front) = 0;
	};


	enum class ClearFlag : uint8_t {
		NONE = 0,
		COLOR = 0b1,
		DEPTH = 0b10,
		STENCIL = 0b100,

		DEPTH_STENCIL = DEPTH | STENCIL,
		ALL = COLOR | DEPTH_STENCIL
	};


	struct AE_FW_DLL DepthState {
		DepthState() :
			enabled(true),
			writeable(true),
			func(ComparisonFunc::LESS),
			reserved(0) {
		}

		inline bool AE_CALL operator==(const DepthState& val) const {
			return featureValue == val.featureValue;
		}

		inline bool AE_CALL operator!=(const DepthState& val) const {
			return featureValue != val.featureValue;
		}

		inline void AE_CALL operator=(const DepthState& val) {
			featureValue = val.featureValue;
		}

		union {
			uint32_t featureValue;

			struct {
				bool enabled;
				bool writeable;
				ComparisonFunc func;
				uint8_t reserved;
			};
		};
	};


	enum class StencilOp : uint8_t {
		KEEP,
		ZERO,
		REPLACE,
		INCR_CLAMP,
		DECR_CLAMP,
		INCR_WRAP,
		DECR_WRAP,
		INVERT
	};


	struct AE_FW_DLL StencilFaceState {
		StencilFaceState() :
			func(ComparisonFunc::ALWAYS),
			op({ StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP}),
			mask({ 0xFF, 0xFF }),
			reserved(0) {
		}

		inline bool AE_CALL operator==(const StencilFaceState& val) const {
			return featureValue == val.featureValue;
		}

		inline bool AE_CALL operator!=(const StencilFaceState& val) const {
			return featureValue != val.featureValue;
		}

		inline void AE_CALL operator=(const StencilFaceState& val) {
			featureValue = val.featureValue;
		}

		union {
			uint64_t featureValue;

			struct {
				ComparisonFunc func;
				struct {
					StencilOp fail;
					StencilOp depthFail;
					StencilOp pass;
				} op;
				struct {
					uint8_t read;
					uint8_t write;
				} mask;

				uint16_t reserved;
			};
		};
	};


	struct AE_FW_DLL StencilState {
		bool enabled = false;
		struct {
			StencilFaceState front;
			StencilFaceState back;
		} face;

		inline bool AE_CALL operator==(const StencilState& val) const {
			return enabled == val.enabled && face.front == val.face.front && face.back == val.face.back;
		}

		inline bool AE_CALL operator!=(const StencilState& val) const {
			return enabled != val.enabled || face.front != val.face.front || face.back != val.face.back;
		}

		inline void AE_CALL operator=(const StencilState& val) {
			enabled = val.enabled;
			face.front = val.face.front;
			face.back = val.face.back;
		}
	};


	class AE_FW_DLL IDepthStencilState : public IObject {
	public:
		IDepthStencilState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IDepthStencilState() {}

		virtual const void* AE_CALL getNative() const = 0;

		virtual const DepthState& AE_CALL getDepthState() const = 0;
		virtual void AE_CALL setDepthState(const DepthState& depthState) = 0;

		virtual const StencilState& AE_CALL getStencilState() const = 0;
		virtual void AE_CALL setStencilState(const StencilState& stencilState) = 0;

	};


	struct AE_FW_DLL GraphicsDeviceFeatures {
		GraphicsDeviceFeatures();

		bool sampler;
		bool nativeTextureView;
		bool nativeRenderView;
		bool pixelBuffer;
		bool constantBuffer;
		bool textureMap;
		bool persistentMap;
		bool independentBlend;
		bool stencilIndependentRef;
		bool stencilIndependentMask;
		uint8_t maxSampleCount;
		uint8_t simultaneousRenderTargetCount;
		std::vector<IndexType> indexTypes;
		std::vector<TextureFormat> textureFormats;

		void AE_CALL reset();
	};


	enum class GraphicsEvent : uint8_t {
		ERR
	};


	class AE_FW_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ModuleType AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> AE_CALL getEventDispatcher() = 0;
		//virtual const events::IEventDispatcher<GraphicsEvent>& AE_CALL getEventDispatcher() const = 0;

		virtual const std::string& AE_CALL getVersion() const = 0;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const = 0;
		virtual IntrusivePtr<IBlendState> AE_CALL createBlendState() = 0;
		virtual IntrusivePtr<IConstantBuffer> AE_CALL createConstantBuffer() = 0;
		virtual IntrusivePtr<IDepthStencil> AE_CALL createDepthStencil() = 0;
		virtual IntrusivePtr<IDepthStencilState> AE_CALL createDepthStencilState() = 0;
		virtual IntrusivePtr<IIndexBuffer> AE_CALL createIndexBuffer() = 0;
		virtual IntrusivePtr<IPixelBuffer> AE_CALL createPixelBuffer() = 0;
		virtual IntrusivePtr<IProgram> AE_CALL createProgram() = 0;
		virtual IntrusivePtr<IRasterizerState> AE_CALL createRasterizerState() = 0;
		virtual IntrusivePtr<IRenderTarget> AE_CALL createRenderTarget() = 0;
		virtual IntrusivePtr<IRenderView> AE_CALL createRenderView() = 0;
		virtual IntrusivePtr<ISampler> AE_CALL createSampler() = 0;
		virtual IntrusivePtr<ITexture1DResource> AE_CALL createTexture1DResource() = 0;
		virtual IntrusivePtr<ITexture2DResource> AE_CALL createTexture2DResource() = 0;
		virtual IntrusivePtr<ITexture3DResource> AE_CALL createTexture3DResource() = 0;
		virtual IntrusivePtr<ITextureView> AE_CALL createTextureView() = 0;
		virtual IntrusivePtr<IVertexBuffer> AE_CALL createVertexBuffer() = 0;

		virtual const Vec2ui32& AE_CALL getBackBufferSize() const = 0;
		virtual void AE_CALL setBackBufferSize(const Vec2ui32& size) = 0;
		virtual Box2i32ui32 AE_CALL getViewport() const = 0;
		virtual void AE_CALL setViewport(const Box2i32ui32& vp) = 0;
		virtual void AE_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) = 0;//unrealized all sampleMask
		virtual void AE_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) = 0;
		inline void AE_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilRef) {
			setDepthStencilState(state, stencilRef, stencilRef);
		}
		virtual void AE_CALL setRasterizerState(IRasterizerState* state) = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) = 0;
		virtual void AE_CALL drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL flush() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL setRenderTarget(IRenderTarget* rt) = 0;
		virtual void AE_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) = 0;
	};
}