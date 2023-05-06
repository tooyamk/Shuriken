#pragma once

#include "srk/modules/IModule.h"
#include "srk/ByteArray.h"
#include "srk/math/Box.h"
#include <functional>
#include <vector>

namespace srk {
	class IShaderParameterGetter;
}

namespace srk::events {
	template<typename EvtType>
	class IEventDispatcher;
}

namespace srk::modules::windows {
	class IWindow;
}

namespace srk::modules::graphics {
	class IGraphicsModule;
	class ITextureResource;
	class GraphicsAdapter;

	using SampleCount = uint8_t;

	inline static constexpr char COMBINED_TEXTURE_SAMPLER_HEADER[14] = { "_combined_ts_" };

	class SRK_FW_DLL IObject : public Ref {
	public:
		virtual ~IObject() {}

		inline const IntrusivePtr<IGraphicsModule>& SRK_CALL getGraphics() const {
			return _graphics;
		}

	protected:
		IObject(IGraphicsModule& graphics);

		IntrusivePtr<IGraphicsModule> _graphics;
	};


	enum class Usage : uint16_t {
		NONE = 0,//create and map
		COPY_SRC = 1 << 0,//create
		COPY_DST = 1 << 1,//create
		MAP_READ = 1 << 2,//create and map
		MAP_WRITE = 1 << 3,//create and map
		UPDATE = 1 << 4,//create

		PERSISTENT_MAP = 1 << 5,//create
		RENDERABLE = 1 << 6,//create

		MAP_SWAP = 1 << 7,//map
		MAP_FORCE_SWAP = (1 << 8) | MAP_SWAP,//map

		DISCARD = 1 << 9,//map return

		COPY_SRC_DST = COPY_SRC | COPY_DST,
		MAP_READ_WRITE = MAP_READ | MAP_WRITE,
		MAP_WRITE_UPDATE = MAP_WRITE | UPDATE,
		BUFFER_CREATE_ALL = COPY_SRC_DST | MAP_READ_WRITE | UPDATE | PERSISTENT_MAP,
		TEXTURE_RESOURCE_CREATE_ALL = COPY_SRC_DST | MAP_READ_WRITE | UPDATE | PERSISTENT_MAP | RENDERABLE
	};


	class SRK_FW_DLL IBuffer : public IObject {
	public:
		IBuffer(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBuffer() {}

		virtual bool SRK_CALL isCreated() const = 0;
		virtual const void* SRK_CALL getNative() const = 0;
		virtual bool SRK_CALL create(size_t size, Usage requiredUsage, Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0) = 0;
		virtual size_t SRK_CALL getSize() const = 0;
		virtual Usage SRK_CALL getUsage() const = 0;
		virtual Usage SRK_CALL map(Usage expectMapUsage) = 0;
		virtual void SRK_CALL unmap() = 0;
		virtual size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) = 0;
		virtual size_t SRK_CALL write(const void* data, size_t length, size_t offset) = 0;
		virtual size_t SRK_CALL update(const void* data, size_t length, size_t offset) = 0;
		virtual size_t SRK_CALL copyFrom(size_t dstPos, const IBuffer* src, const Box1uz& srcRange) = 0;
		//virtual void SRK_CALL flush() = 0;
		virtual bool SRK_CALL isSyncing() const = 0;
		virtual void SRK_CALL destroy() = 0;
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


	struct SRK_FW_DLL VertexFormat {
		VertexFormat() :
			dimension(0),
			type(VertexType::UNKNOWN) {
		}

		VertexFormat(const VertexFormat& other) :
			featureValue(other.featureValue) {
		}

		VertexFormat(VertexFormat&& other) noexcept :
			featureValue(other.featureValue) {
		}

		VertexFormat(uint8_t dim, VertexType type) :
			dimension(dim),
			type(type) {
		}

		inline bool SRK_CALL operator==(const VertexFormat& val) const {
			return featureValue == val.featureValue;
		}

		inline bool SRK_CALL operator!=(const VertexFormat& val) const {
			return featureValue != val.featureValue;
		}

		inline void SRK_CALL operator=(const VertexFormat& val) {
			featureValue = val.featureValue;
		}

		inline void SRK_CALL operator=(VertexFormat&& val) noexcept {
			featureValue = val.featureValue;
		}

		template<typename Type>
		inline void SRK_CALL set() {
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

		template<typename Type>
		inline void SRK_CALL set(uint8_t dim) {
			dimension = dim;
			set<Type>();
		}

		inline void SRK_CALL set(uint8_t dim, VertexType type) {
			dimension = dim;
			this->type = type;
		}

		union {
			uint16_t featureValue;

			struct {
				uint8_t dimension;
				VertexType type;
			};
		};
	};


	class SRK_FW_DLL IVertexBuffer : public IBuffer {
	public:
		IVertexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IVertexBuffer() {}

		virtual size_t SRK_CALL getStride() const = 0;
		virtual void SRK_CALL setStride(size_t stride) = 0;
	};


	struct SRK_FW_DLL VertexAttributeDescriptor {
		VertexAttributeDescriptor() :
			offset(0) {
		}

		VertexAttributeDescriptor(const VertexFormat& format, size_t offset) :
			format(format),
			offset(offset) {
		}

		VertexAttributeDescriptor(const VertexAttributeDescriptor& other) :
			format(other.format),
			offset(other.offset) {
		}

		VertexAttributeDescriptor(VertexAttributeDescriptor&& other) noexcept :
			format(other.format),
			offset(other.offset) {
		}

		inline void SRK_CALL operator=(const VertexAttributeDescriptor& other) {
			format = other.format;
			offset = other.offset;
		}

		inline void SRK_CALL operator=(VertexAttributeDescriptor&& other) noexcept {
			format = other.format;
			offset = other.offset;
		}

		VertexFormat format;
		size_t offset;
	};


	template<IntrusivePtrOperableObject T>
	struct VertexAttribute {
		VertexAttribute() {}

		VertexAttribute(T* resource, const VertexAttributeDescriptor& desc) :
			resource(resource),
			desc(desc) {
		}

		VertexAttribute(T* resource, uint8_t dim, VertexType type, uint32_t offset) : VertexAttribute(resource, VertexAttributeDescriptor(VertexFormat(dim, type), offset)) {}

		VertexAttribute(const VertexAttribute& other) :
			resource(other.resource),
			desc(other.desc) {
		}

		VertexAttribute(VertexAttribute&& other) noexcept :
			resource(std::move(other.resource)),
			desc(other.desc) {
		}

		inline void SRK_CALL operator=(const VertexAttribute& other) {
			resource = other.resource;
			desc = other.desc;
		}

		inline void SRK_CALL operator=(VertexAttribute&& other) noexcept {
			resource = std::move(other.resource);
			desc = other.desc;
		}

		IntrusivePtr<T> resource;
		VertexAttributeDescriptor desc;
	};


	class SRK_FW_DLL IVertexAttributeGetter : public Ref {
	public:
		virtual ~IVertexAttributeGetter() {}

		virtual std::optional<VertexAttribute<IVertexBuffer>> SRK_CALL get(const std::string_view& name) const = 0;
	};


	enum class IndexType : uint8_t {
		UNKNOWN,
		UI8,
		UI16,
		UI32
	};


	class SRK_FW_DLL IIndexBuffer : public IBuffer {
	public:
		IIndexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IIndexBuffer() {}

		virtual IndexType SRK_CALL getFormat() const = 0;
		virtual void SRK_CALL setFormat(IndexType type) = 0;

		template<typename T>
		inline void SRK_CALL setFormat() {
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


	class SRK_FW_DLL IConstantBuffer : public IBuffer {
	public:
		virtual ~IConstantBuffer() {}

	protected:
		IConstantBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
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
		REPEAT,
		CLAMP_EDGE,
		CLAMP_BORDER,
		MIRROR_REPEAT,
		MIRROR_CLAMP_EDGE
	};


	class SRK_FW_DLL SamplerFilter {
	public:
		using FeatureValueType = uint32_t;

		SamplerFilter(SamplerFilterOperation op = SamplerFilterOperation::NORMAL, SamplerFilterMode min = SamplerFilterMode::LINEAR, SamplerFilterMode mag = SamplerFilterMode::LINEAR, SamplerFilterMode mipmap = SamplerFilterMode::LINEAR);
		SamplerFilter(const SamplerFilter& filter);
		SamplerFilter(SamplerFilter&& filter) noexcept;

		inline bool SRK_CALL operator==(const SamplerFilter& val) const {
			return getFeatureValue() == val.getFeatureValue();
		}

		inline bool SRK_CALL operator!=(const SamplerFilter& val) const {
			return getFeatureValue() != val.getFeatureValue();
		}

		inline void SRK_CALL operator=(const SamplerFilter& val) {
			*(FeatureValueType*)this = val.getFeatureValue();
		}

		inline SamplerFilter& SRK_CALL operator=(SamplerFilter&& val) noexcept {
			*(FeatureValueType*)this = val.getFeatureValue();
			return *this;
		}

		inline FeatureValueType SRK_CALL getFeatureValue() const {
			return *(FeatureValueType*)this;
		}

		SamplerFilterOperation operation;
		SamplerFilterMode minification;
		SamplerFilterMode magnification;
		SamplerFilterMode mipmap;
	};


	class SRK_FW_DLL SamplerAddress {
	public:
		using FeatureValueType = uint32_t;

		SamplerAddress(SamplerAddressMode u = SamplerAddressMode::REPEAT, SamplerAddressMode v = SamplerAddressMode::REPEAT, SamplerAddressMode w = SamplerAddressMode::REPEAT);
		SamplerAddress(const SamplerAddress& address);
		SamplerAddress(SamplerAddress&& address) noexcept;

		inline bool SRK_CALL operator==(const SamplerAddress& val) const {
			return getFeatureValue() == val.getFeatureValue();
		}

		inline bool SRK_CALL operator!=(const SamplerAddress& val) const {
			return getFeatureValue() != val.getFeatureValue();
		}

		inline void SRK_CALL operator=(const SamplerAddress& val) {
			*(FeatureValueType*)this = val.getFeatureValue();
		}

		inline SamplerAddress& SRK_CALL operator=(SamplerAddress&& val) noexcept {
			*(FeatureValueType*)this = val.getFeatureValue();
			return *this;
		}

		inline FeatureValueType SRK_CALL getFeatureValue() const {
			return *(FeatureValueType*)this;
		}

		SamplerAddressMode u;
		SamplerAddressMode v;
		SamplerAddressMode w;
		uint8_t reserved;
	};


	class SRK_FW_DLL ISampler : public IObject {
	public:
		ISampler(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ISampler() {}

		virtual const void* SRK_CALL getNative() const = 0;

		inline void SRK_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) {
			setFilter(SamplerFilter(op, min, mag, mipmap));
		}
		virtual void SRK_CALL setFilter(const SamplerFilter& filter) = 0;

		virtual void SRK_CALL setComparisonFunc(ComparisonFunc func) = 0;

		inline void SRK_CALL setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) {
			setAddress(SamplerAddress(u, v, w));
		}
		inline void SRK_CALL setAddress(SamplerAddressMode mode) {
			setAddress(SamplerAddress(mode, mode, mode));
		}
		virtual void SRK_CALL setAddress(const SamplerAddress& address) = 0;

		virtual void SRK_CALL setMipLOD(float32_t min, float32_t max) = 0;
		virtual void SRK_CALL setMipLODBias(float32_t bias) = 0;
		virtual void SRK_CALL setMaxAnisotropy(uint32_t max) = 0;

		virtual void SRK_CALL setBorderColor(const Vec4f32& color) = 0;
	};


	enum class TextureType : uint8_t {
		TEX1D,
		TEX2D,
		TEX3D
	};


	enum class TextureFormat : uint8_t {
		UNKNOWN,
		R8G8B8_TYPELESS,
		R8G8B8_UNORM,
		R8G8B8_UNORM_SRGB,
		R8G8B8_UINT,
		R8G8B8_SNORM,
		R8G8B8_SINT,
		R8G8B8A8_TYPELESS,
		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,
		BC1_TYPELESS,
		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC2_TYPELESS,
		BC2_UNORM,
		BC2_UNORM_SRGB,
		BC3_TYPELESS,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_TYPELESS,
		BC4_UNORM,
		BC4_SNORM,
		BC5_TYPELESS,
		BC5_UNORM,
		BC5_SNORM,
		BC6_TYPELESS,
		BC6H_UF16,
		BC6H_SF16,
		BC7_TYPELESS,
		BC7_UNORM,
		BC7_UNORM_SRGB
	};


	class SRK_FW_DLL TextureUtility {
	public:
		TextureUtility() = delete;

		static bool SRK_CALL isCompressedFormat(TextureFormat format);

		static size_t SRK_CALL getBlocks(TextureFormat format, size_t pixels);
		static Vec2uz SRK_CALL getBlocks(TextureFormat format, const Vec2uz& pixels);
		static size_t SRK_CALL getPerBlockBytes(TextureFormat format);
		static Vec2uz SRK_CALL getPerBlockPixels(TextureFormat format);

		inline static size_t SRK_CALL getMipLevels(size_t n) {
			return (size_t)std::floor(std::log2(n) + 1);
		}
		inline static size_t SRK_CALL getMipLevels(const Vec2uz& pixels) {
			return getMipLevels(pixels.getMax());
		}
		inline static size_t SRK_CALL getMipLevels(const Vec3uz& pixels) {
			return getMipLevels(pixels.getMax());
		}

		inline static size_t SRK_CALL getBytes(TextureFormat format, const Vec2uz& pixels) {
			return getBytes(getPerBlockBytes(format), getBlocks(format, pixels).getMultiplies());
		}
		inline static size_t SRK_CALL getBytes(TextureFormat format, size_t blocks) {
			return getBytes(getPerBlockBytes(format), blocks);
		}
		inline static constexpr size_t SRK_CALL getBytes(size_t perBlockBytes, size_t blocks) {
			return blocks * perBlockBytes;
		}

		inline static size_t SRK_CALL getNextMipPixels(size_t n) {
			return n > 1 ? n >> 1 : 1;
		}
		inline static Vec2uz SRK_CALL getNextMipPixels(const Vec2uz& pixels) {
			return Vec2uz(getNextMipPixels(pixels[0]), getNextMipPixels(pixels[1]));
		}
		inline static Vec3uz SRK_CALL getNextMipPixels(const Vec3uz& pixels) {
			return Vec3uz(getNextMipPixels(pixels[0]), getNextMipPixels(pixels[1]), getNextMipPixels(pixels[2]));
		}

		template<SameAnyOf<Vec2uz, Vec3uz> T>
		static void SRK_CALL getMipsInfo(TextureFormat format, const T& pixels, size_t mipLevels, size_t* totalBytes = nullptr, size_t* mipBytes = nullptr, T* mipDimensions = nullptr) {
			auto size = pixels;
			auto bytes = getBytes(format, pixels);
			if (totalBytes) *totalBytes = bytes;
			if (mipBytes) mipBytes[0] = bytes;
			if (mipDimensions) mipDimensions[0] = size;

			for (size_t i = 1; i < mipLevels; ++i) {
				size = getNextMipPixels(size);
				bytes = getBytes(format, size);
				if (totalBytes) *totalBytes += bytes;
				if (mipBytes) mipBytes[i] = bytes;
				if (mipDimensions) mipDimensions[i] = size;
			}
		}
	};


	class SRK_FW_DLL ITextureResource : public IObject {
	public:
		ITextureResource(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ITextureResource() {}

		virtual TextureType SRK_CALL getType() const = 0;
		virtual bool SRK_CALL isCreated() const = 0;
		virtual const void* SRK_CALL getNative() const = 0;
		virtual SampleCount SRK_CALL getSampleCount() const = 0;
		virtual TextureFormat SRK_CALL getFormat() const = 0;
		virtual Usage SRK_CALL getUsage() const = 0;
		virtual const Vec3uz& SRK_CALL getDimensions() const = 0;
		virtual bool SRK_CALL create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data = nullptr) = 0;
		virtual Usage SRK_CALL map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) = 0;
		virtual void SRK_CALL unmap(size_t arraySlice, size_t mipSlice) = 0;
		virtual size_t SRK_CALL read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) = 0;
		virtual size_t SRK_CALL write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) = 0;
		virtual bool SRK_CALL copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) = 0;
		virtual void SRK_CALL destroy() = 0;
	};


	class SRK_FW_DLL ITexture1DResource : public ITextureResource {
	public:
		ITexture1DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture1DResource() {}

		virtual bool SRK_CALL create(size_t width, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data = nullptr) = 0;
		virtual bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box1uz& range, const void* data) = 0;

		virtual bool SRK_CALL create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data = nullptr) override {
			return create(dim[0], arraySize, mipLevels, format, requiredUsage, preferredUsage, data);
		}
	};


	class SRK_FW_DLL ITexture2DResource : public ITextureResource {
	public:
		ITexture2DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture2DResource() {}

		virtual bool SRK_CALL create(const Vec2uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data = nullptr) = 0;
		virtual bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box2uz& range, const void* data) = 0;

		virtual bool SRK_CALL create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data = nullptr) override {
			return create((const Vec2uz&)dim, arraySize, mipLevels, sampleCount, format, requiredUsage, preferredUsage, data);
		}
	};


	class SRK_FW_DLL ITexture3DResource : public ITextureResource {
	public:
		ITexture3DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture3DResource() {}

		virtual bool SRK_CALL create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data = nullptr) = 0;
		virtual bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) = 0;

		virtual bool SRK_CALL create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data = nullptr) override {
			return create(dim, arraySize, mipLevels, format, requiredUsage, preferredUsage, data);
		}
	};


	class SRK_FW_DLL ITextureView : public IObject {
	public:
		ITextureView(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ITextureView() {}

		//virtual TextureType SRK_CALL getType() const = 0;
		virtual bool SRK_CALL isCreated() const = 0;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const = 0;
		virtual const void* SRK_CALL getNative() const = 0;
		virtual size_t SRK_CALL getArraySize() const = 0;
		virtual size_t SRK_CALL getMipLevels() const = 0;
		virtual bool SRK_CALL create(ITextureResource* res, size_t mipBegin, size_t mipLevels, size_t arrayBegin, size_t arraySize) = 0;
		virtual void SRK_CALL destroy() = 0;
	};


	class SRK_FW_DLL IRenderView : public IObject {
	public:
		IRenderView(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRenderView() {}

		virtual bool SRK_CALL isCreated() const = 0;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const = 0;
		virtual const void* SRK_CALL getNative() const = 0;
		virtual size_t SRK_CALL getArraySize() const = 0;
		virtual size_t SRK_CALL getMipSlice() const = 0;
		virtual bool SRK_CALL create(ITextureResource* res, size_t mipSlice, size_t arrayBegin, size_t arraySize) = 0;
		virtual void SRK_CALL destroy() = 0;
	};


	enum class DepthStencilFormat : uint8_t {
		UNKNOWN,
		D16,
		D24,
		D24S8,
		D32,
		D32S8
	};


	class SRK_FW_DLL IDepthStencil : public IObject {
	public:
		IDepthStencil(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IDepthStencil() {}

		virtual const void* SRK_CALL getNative() const = 0;
		virtual SampleCount SRK_CALL getSampleCount() const = 0;
		virtual const Vec2uz& SRK_CALL getSize() const = 0;
		virtual bool SRK_CALL create(const Vec2uz& size, DepthStencilFormat format, SampleCount sampleCount) = 0;
		virtual void SRK_CALL destroy() = 0;
	};


	class SRK_FW_DLL IRenderTarget : public IObject {
	public:
		IRenderTarget(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRenderTarget() {}

		virtual const void* SRK_CALL getNative() const = 0;

		virtual Vec2uz SRK_CALL getDimensions() const = 0;
		virtual IntrusivePtr<IRenderView> SRK_CALL getRenderView(uint8_t index) const = 0;
		virtual bool SRK_CALL setRenderView(uint8_t index, IRenderView* view) = 0;
		virtual void SRK_CALL eraseRenderViews(uint8_t begin, uint8_t count) = 0;

		virtual IntrusivePtr<IDepthStencil> SRK_CALL getDepthStencil() const = 0;
		virtual void SRK_CALL setDepthStencil(IDepthStencil* ds) = 0;
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


	class SRK_FW_DLL BlendFunc {
	public:
		using FeatureValueType = uint32_t;

		BlendFunc();
		BlendFunc(nullptr_t);
		BlendFunc(BlendFactor src, BlendFactor dst);
		BlendFunc(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);
		BlendFunc(const BlendFunc& func);

		inline bool SRK_CALL operator==(const BlendFunc& val) const {
			return getFeatureValue() == val.getFeatureValue();
		}

		inline bool SRK_CALL operator!=(const BlendFunc& val) const {
			return getFeatureValue() != val.getFeatureValue();
		}

		inline void SRK_CALL operator=(const BlendFunc& val) {
			*(FeatureValueType*)this = val.getFeatureValue();
		}

		inline BlendFunc& SRK_CALL operator=(BlendFunc&& val) noexcept {
			*(FeatureValueType*)this = val.getFeatureValue();
			return *this;
		}
		
		inline BlendFunc& SRK_CALL set(BlendFactor src, BlendFactor dst) {
			return set(src, dst, src, dst);
		}

		inline BlendFunc& SRK_CALL set(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) {
			this->srcColor = srcColor;
			this->dstColor = dstColor;
			this->srcAlpha = srcAlpha;
			this->dstAlpha = dstAlpha;
			
			return *this;
		}

		inline FeatureValueType SRK_CALL getFeatureValue() const {
			return *(FeatureValueType*)this;
		}

		BlendFactor srcColor;
		BlendFactor dstColor;
		BlendFactor srcAlpha;
		BlendFactor dstAlpha;
	};


	class SRK_FW_DLL BlendEquation {
	public:
		using FeatureValueType = uint16_t;

		BlendEquation();
		BlendEquation(nullptr_t);
		BlendEquation(BlendOp op);
		BlendEquation(BlendOp color, BlendOp alpha);
		BlendEquation(const BlendEquation& op);
		BlendEquation(BlendEquation&& op) noexcept;

		inline bool SRK_CALL operator==(const BlendEquation& val) const {
			return getFeatureValue() == val.getFeatureValue();
		}

		inline bool SRK_CALL operator!=(const BlendEquation& val) const {
			return getFeatureValue() != val.getFeatureValue();
		}

		inline void SRK_CALL operator=(const BlendEquation& val) {
			*(FeatureValueType*)this = val.getFeatureValue();
		}

		inline BlendEquation& SRK_CALL operator=(BlendEquation&& val) noexcept {
			*(FeatureValueType*)this = val.getFeatureValue();
			return *this;
		}

		inline void SRK_CALL operator=(BlendOp op) {
			set(op);
		}

		inline BlendEquation& SRK_CALL set(BlendOp op) {
			return set(op, op);
		}

		inline BlendEquation& SRK_CALL set(BlendOp color, BlendOp alpha) {
			this->color = color;
			this->alpha = alpha;

			return *this;
		}

		inline FeatureValueType SRK_CALL getFeatureValue() const {
			return *(FeatureValueType*)this;
		}

		BlendOp color;
		BlendOp alpha;
	};


	class SRK_FW_DLL RenderTargetBlendState {
	public:
		using FeatureValueType = uint64_t;

		RenderTargetBlendState();
		RenderTargetBlendState(const RenderTargetBlendState& other);
		RenderTargetBlendState(RenderTargetBlendState&& other) noexcept;

		inline bool SRK_CALL operator==(const RenderTargetBlendState& val) const {
			return getFeatureValue() == val.getFeatureValue();
		}

		inline bool SRK_CALL operator!=(const RenderTargetBlendState& val) const {
			return getFeatureValue() != val.getFeatureValue();
		}

		inline void SRK_CALL operator=(const RenderTargetBlendState& val) {
			*(FeatureValueType*)this = val.getFeatureValue();
		}

		inline RenderTargetBlendState& SRK_CALL operator=(RenderTargetBlendState&& val) noexcept {
			*(FeatureValueType*)this = val.getFeatureValue();
			return *this;
		}

		inline FeatureValueType SRK_CALL getFeatureValue() const {
			return *(FeatureValueType*)this;
		}

		bool enabled;
		BlendEquation equation;
		BlendFunc func;
		Vec4<bool> writeMask;
	};


	class SRK_FW_DLL IBlendState : public IObject {
	public:
		IBlendState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBlendState() {}

		virtual const void* SRK_CALL getNative() const = 0;
		virtual const Vec4f32& SRK_CALL getConstants() const = 0;
		virtual void SRK_CALL setConstants(const Vec4f32& val) = 0;
		virtual uint8_t SRK_CALL getCount() const = 0;
		virtual void SRK_CALL setCount(uint8_t count) = 0;

		virtual const RenderTargetBlendState* SRK_CALL getRenderTargetState(uint8_t index) const = 0;
		virtual bool SRK_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) = 0;
	};


	enum class ProgramStage : uint8_t {
		UNKNOWN,
		CS,//ComputeShader
		DS, //DomainShader
		GS,//GeomtryShader
		HS,//HullShader
		PS,//PixelShader
		VS,//VertexShader
	};


	enum class ProgramLanguage : uint8_t {
		UNKNOWN,
		HLSL,
		DXIL,
		SPIRV,
		GLSL,
		GSSL,
		MSL
	};


	class SRK_FW_DLL ProgramSource : public Ref {
	public:
		ProgramSource();
		ProgramSource(ProgramSource&& value);

		ProgramLanguage language;
		ProgramStage stage;
		std::string version;
		std::string entryPoint;
		ByteArray data;

		inline static const std::string defaultEntryPoint = std::string("main");

		ProgramSource& SRK_CALL operator=(ProgramSource&& value) noexcept;

		bool SRK_CALL isValid() const;

		inline std::string_view toHLSLShaderStage() const {
			return toHLSLShaderStage(stage);
		}

		static std::string_view toHLSLShaderStage(ProgramStage stage);

		inline std::string SRK_CALL toHLSLShaderModel() const {
			return toHLSLShaderModel(stage, version);
		}

		static std::string SRK_CALL toHLSLShaderModel(ProgramStage stage, const std::string_view& version);

		inline const std::string& SRK_CALL getEntryPoint() const {
			return getEntryPoint(entryPoint);
		}
		inline static std::string_view SRK_CALL getEntryPoint(const std::string_view& entryPoint) {
			return entryPoint.empty() ? defaultEntryPoint : entryPoint;
		}
		inline static const std::string& SRK_CALL getEntryPoint(const std::string& entryPoint) {
			return entryPoint.empty() ? defaultEntryPoint : entryPoint;
		}
	};


	class SRK_FW_DLL ProgramInfo {
	public:
		class SRK_FW_DLL Vertex {
		public:
			std::string name;
			VertexFormat format;
			bool instanced = false;
		};


		std::vector<Vertex> vertices;

		inline void SRK_CALL clear() {
			vertices.clear();
		}
	};


	struct SRK_FW_DLL ProgramInputInfo {
		std::string_view name;
	};


	struct SRK_FW_DLL ProgramInputDescriptor {
		bool instanced = false;
	};


	using ProgramInputHandler = std::function<ProgramInputDescriptor(const ProgramInputInfo&)>;


	struct SRK_FW_DLL ProgramIncludeInfo {
		std::string_view file;
	};


	using ProgramIncludeHandler = std::function<ByteArray(const ProgramIncludeInfo&)>;


	class SRK_FW_DLL ProgramDefine {
	public:
		ProgramDefine() {}

		ProgramDefine(const std::string_view& name, const std::string_view& value) :
			name(name),
			value(value) {}

		std::string_view name;
		std::string_view value;
	};


	struct SRK_FW_DLL ProgramTranspileInfo {
		const ProgramSource* source;
		ProgramLanguage targetLanguage;
		std::string_view targetVersion;
		const ProgramDefine* defines;
		size_t numDefines;
		ProgramIncludeHandler includeHandler;
	};


	using ProgramTranspileHandler = std::function<ProgramSource(const ProgramTranspileInfo&)>;


	class SRK_FW_DLL IProgram : public IObject {
	public:
		IProgram(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IProgram() {}

		virtual const void* SRK_CALL getNative() const = 0;
		virtual bool SRK_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ProgramDefine* defines, size_t numDefines, const ProgramIncludeHandler& includeHandler, const ProgramInputHandler& inputHandler, const ProgramTranspileHandler& transpileHandler) = 0;
		virtual const ProgramInfo& getInfo() const = 0;
		virtual void SRK_CALL destroy() = 0;
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


	struct SRK_FW_DLL RasterizerDescriptor {
		RasterizerDescriptor();

		bool scissorEnabled;
		FillMode fillMode;
		CullMode cullMode;
		FrontFace frontFace;
	};


	class SRK_FW_DLL IRasterizerState : public IObject {
	public:
		IRasterizerState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IRasterizerState() {}

		virtual const void* SRK_CALL getNative() const = 0;

		virtual FillMode SRK_CALL getFillMode() const = 0;
		virtual void SRK_CALL setFillMode(FillMode fill) = 0;

		virtual CullMode SRK_CALL getCullMode() const = 0;
		virtual void SRK_CALL setCullMode(CullMode cull) = 0;

		virtual FrontFace SRK_CALL getFrontFace() const = 0;
		virtual void SRK_CALL setFrontFace(FrontFace front) = 0;

		virtual bool SRK_CALL getScissorEnabled() const = 0;
		virtual void SRK_CALL setScissorEnabled(bool enabled) = 0;
	};


	class SRK_FW_DLL RasterizerFeature {
	public:
		RasterizerFeature() noexcept;
		RasterizerFeature(const RasterizerFeature& other) noexcept;

		void SRK_CALL set(const IRasterizerState& state);
		void SRK_CALL set(const RasterizerDescriptor& desc) noexcept;
		bool SRK_CALL trySet(const RasterizerFeature& val) noexcept;

		inline bool SRK_CALL operator==(const RasterizerFeature& val) const noexcept {
			return _value == val._value;
		}

		inline bool SRK_CALL operator!=(const RasterizerFeature& val) const noexcept {
			return _value != val._value;
		}

		inline constexpr void SRK_CALL operator=(const RasterizerFeature& val) noexcept {
			_value = val._value;
		}

		inline constexpr void SRK_CALL operator=(nullptr_t) noexcept {
			_value = 0;
		}

	private:
		uint32_t _value;
	};


	enum class ClearFlag : uint8_t {
		NONE = 0,
		COLOR = 0b1,
		DEPTH = 0b10,
		STENCIL = 0b100,

		DEPTH_STENCIL = DEPTH | STENCIL,
		ALL = COLOR | DEPTH_STENCIL
	};


	struct SRK_FW_DLL DepthState {
		DepthState();

		inline bool SRK_CALL operator==(const DepthState& val) const {
			return featureValue == val.featureValue;
		}

		inline bool SRK_CALL operator!=(const DepthState& val) const {
			return featureValue != val.featureValue;
		}

		inline void SRK_CALL operator=(const DepthState& val) {
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


	struct SRK_FW_DLL StencilFaceState {
		StencilFaceState();

		inline bool SRK_CALL operator==(const StencilFaceState& val) const {
			return featureValue == val.featureValue;
		}

		inline bool SRK_CALL operator!=(const StencilFaceState& val) const {
			return featureValue != val.featureValue;
		}

		inline void SRK_CALL operator=(const StencilFaceState& val) {
			featureValue = val.featureValue;
		}

		union {
			uint64_t featureValue;

			struct {
				union {
					uint32_t funcAndOpFeatureValue;

					struct {
						ComparisonFunc func;

						struct {
							StencilOp depthFail;
							StencilOp fail;
							StencilOp pass;
						} op;
					};
				};

				union {
					uint16_t featureValue;

					struct {
						uint8_t read;
						uint8_t write;
					};
				} mask;

				uint8_t ref;
				uint8_t reserved;
			};
		};
	};


	struct SRK_FW_DLL StencilState {
		bool enabled = false;
		struct {
			StencilFaceState front;
			StencilFaceState back;
		} face;

		inline bool SRK_CALL operator==(const StencilState& val) const {
			return enabled == val.enabled && face.front == val.face.front && face.back == val.face.back;
		}

		inline bool SRK_CALL operator!=(const StencilState& val) const {
			return enabled != val.enabled || face.front != val.face.front || face.back != val.face.back;
		}

		inline void SRK_CALL operator=(const StencilState& val) {
			enabled = val.enabled;
			face.front = val.face.front;
			face.back = val.face.back;
		}
	};


	class SRK_FW_DLL IDepthStencilState : public IObject {
	public:
		IDepthStencilState(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IDepthStencilState() {}

		virtual const void* SRK_CALL getNative() const = 0;

		virtual const DepthState& SRK_CALL getDepthState() const = 0;
		virtual void SRK_CALL setDepthState(const DepthState& depthState) = 0;

		virtual const StencilState& SRK_CALL getStencilState() const = 0;
		virtual void SRK_CALL setStencilState(const StencilState& stencilState) = 0;
	};


	class SRK_FW_DLL DepthStencilFeature {
	public:
		DepthStencilFeature() noexcept;
		DepthStencilFeature(const DepthStencilFeature& other) noexcept;

		void SRK_CALL set(const DepthState& depth, const StencilState& stencil);
		void SRK_CALL set(const StencilState& stencil);
		bool SRK_CALL trySet(const DepthStencilFeature& val) noexcept;

		inline bool SRK_CALL operator==(const DepthStencilFeature& val) const noexcept {
			return _low == val._low && _high == val._high;
		}

		inline bool SRK_CALL operator!=(const DepthStencilFeature& val) const noexcept {
			return _low != val._low || _high != val._high;
		}

		inline constexpr void SRK_CALL operator=(const DepthStencilFeature& val) noexcept {
			_low = val._low;
			_high = val._high;
		}

		inline constexpr void SRK_CALL operator=(nullptr_t) noexcept {
			_low = 0;
			_high = 0;
		}

	private:
		uint64_t _low;
		uint64_t _high;

		void SRK_CALL _setValid() noexcept;
		void SRK_CALL _set(const StencilState& stencil);
	};


	class SRK_FW_DLL ICommandBuffer : public Ref {
	public:
		virtual void SRK_CALL setViewport(const Box2i32ui32& vp) = 0;
	};


	struct SRK_FW_DLL GraphicsDeviceFeatures {
		GraphicsDeviceFeatures();

		bool sampler;
		bool nativeTextureView;
		bool nativeRenderView;
		bool constantBuffer;
		bool textureMap;
		bool persistentMap;
		bool independentBlend;
		bool stencilIndependentRef;
		bool stencilIndependentMask;
		bool vertexDim3Bit8;
		bool vertexDim3Bit16;
		uint8_t maxSampleCount;
		uint8_t simultaneousRenderTargetCount;
		uint8_t maxSamplerAnisotropy;
		std::vector<IndexType> indexTypes;
		std::vector<TextureFormat> textureFormats;
		std::vector<SamplerAddressMode> samplerAddressModes;

		void SRK_CALL reset();
	};


	enum class GraphicsEvent : uint8_t {
		ERR
	};


	class SRK_FW_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ModuleType SRK_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> SRK_CALL getEventDispatcher() = 0;
		//virtual const events::IEventDispatcher<GraphicsEvent>& SRK_CALL getEventDispatcher() const = 0;

		virtual std::string_view SRK_CALL getVersion() const = 0;
		virtual const GraphicsDeviceFeatures& SRK_CALL getDeviceFeatures() const = 0;
		virtual IntrusivePtr<IBlendState> SRK_CALL createBlendState() = 0;
		virtual IntrusivePtr<IConstantBuffer> SRK_CALL createConstantBuffer() = 0;
		virtual IntrusivePtr<IDepthStencil> SRK_CALL createDepthStencil() = 0;
		virtual IntrusivePtr<IDepthStencilState> SRK_CALL createDepthStencilState() = 0;
		virtual IntrusivePtr<IIndexBuffer> SRK_CALL createIndexBuffer() = 0;
		virtual IntrusivePtr<IProgram> SRK_CALL createProgram() = 0;
		virtual IntrusivePtr<IRasterizerState> SRK_CALL createRasterizerState() = 0;
		virtual IntrusivePtr<IRenderTarget> SRK_CALL createRenderTarget() = 0;
		virtual IntrusivePtr<IRenderView> SRK_CALL createRenderView() = 0;
		virtual IntrusivePtr<ISampler> SRK_CALL createSampler() = 0;
		virtual IntrusivePtr<ITexture1DResource> SRK_CALL createTexture1DResource() = 0;
		virtual IntrusivePtr<ITexture2DResource> SRK_CALL createTexture2DResource() = 0;
		virtual IntrusivePtr<ITexture3DResource> SRK_CALL createTexture3DResource() = 0;
		virtual IntrusivePtr<ITextureView> SRK_CALL createTextureView() = 0;
		virtual IntrusivePtr<IVertexBuffer> SRK_CALL createVertexBuffer() = 0;

		virtual const Vec2ui32& SRK_CALL getBackBufferSize() const = 0;
		virtual void SRK_CALL setBackBufferSize(const Vec2ui32& size) = 0;
		virtual Box2i32ui32 SRK_CALL getViewport() const = 0;
		virtual void SRK_CALL setViewport(const Box2i32ui32& vp) = 0;
		virtual Box2i32ui32 SRK_CALL getScissor() const = 0;
		virtual void SRK_CALL setScissor(const Box2i32ui32& scissor) = 0;
		virtual void SRK_CALL setBlendState(IBlendState* state, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) = 0;//unrealized all sampleMask
		virtual void SRK_CALL setDepthStencilState(IDepthStencilState* state) = 0;
		virtual void SRK_CALL setRasterizerState(IRasterizerState* state) = 0;

		virtual void SRK_CALL beginRender() = 0;
		virtual void SRK_CALL draw(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) = 0;
		virtual void SRK_CALL drawInstanced(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) = 0;
		virtual void SRK_CALL endRender() = 0;
		virtual void SRK_CALL flush() = 0;
		virtual void SRK_CALL present() = 0;

		virtual void SRK_CALL setRenderTarget(IRenderTarget* rt) = 0;
		virtual void SRK_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) = 0;
	};


	enum class DriverType : uint8_t {
		UNKNOWN,
		HARDWARE,
		SOFTWARE
	};


	class SRK_FW_DLL CreateGrahpicsModuleDescriptor {
	public:
		bool debug = false;
		bool offscreen = false;
		DriverType driverType = DriverType::UNKNOWN;
		windows::IWindow* window = nullptr;
		GraphicsAdapter* adapter = nullptr;
		SampleCount sampleCount = 1;
		std::function<void(const std::string_view&)> createProcessInfoHandler = nullptr;
	};
}