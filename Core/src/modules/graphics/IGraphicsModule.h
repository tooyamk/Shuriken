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


	enum class Usage : ui8 {
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
		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) = 0;
		virtual ui32 AE_CALL getSize() const = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap() = 0;
		virtual ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) = 0;
		virtual ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) = 0;
		//virtual void AE_CALL flush() = 0;
		virtual bool AE_CALL isSyncing() const = 0;
	};


	enum class VertexSize : ui8 {
		UNKNOWN,
		ONE,
		TWO,
		THREE,
		FOUR
	};


	enum class VertexType : ui8 {
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


	enum class IndexType : ui8 {
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


	enum class SamplerFilterMode : ui8 {
		POINT,
		LINEAR,
		ANISOTROPIC
	};


	enum class SamplerFilterOperation : ui8 {
		NORMAL,
		COMPARISON,
		MINIMUM,
		MAXIMUM
	};


	enum class SamplerComparisonFunc : ui8 {
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};


	enum class SamplerAddressMode : ui8 {
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE
	};


	struct AE_DLL SamplerFilter {
		SamplerFilter();
		SamplerFilterOperation operation;
		SamplerFilterMode minification;
		SamplerFilterMode magnification;
		SamplerFilterMode mipmap;
	};


	struct AE_DLL SamplerAddress {
		SamplerAddress(SamplerAddressMode u = SamplerAddressMode::WRAP, SamplerAddressMode v = SamplerAddressMode::WRAP, SamplerAddressMode w = SamplerAddressMode::WRAP);

		SamplerAddressMode u;
		SamplerAddressMode v;
		SamplerAddressMode w;
	};


	class AE_DLL ISampler : public IObject {
	public:
		ISampler(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~ISampler() {}

		virtual void AE_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) = 0;
		inline void AE_CALL setFilter(const SamplerFilter& filter) {
			setFilter(filter.operation, filter.minification, filter.magnification, filter.mipmap);
		}

		virtual void AE_CALL setComparisonFunc(SamplerComparisonFunc func) = 0;

		virtual void AE_CALL setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) = 0;
		inline void AE_CALL setAddress(const SamplerAddress& address) {
			setAddress(address.u, address.v, address.w);
		}

		virtual void AE_CALL setMipLOD(f32 min, f32 max) = 0;
		virtual void AE_CALL setMipLODBias(f32 bias) = 0;
		virtual void AE_CALL setMaxAnisotropy(ui32 max) = 0;

		virtual void AE_CALL setBorderColor(const Vec4f32& color) = 0;
	};


	enum class TextureType : ui8 {
		TEX1D,
		TEX2D,
		TEX3D
	};


	enum class TextureFormat : ui8 {
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
		virtual ui32 AE_CALL getArraySize() const = 0;
		virtual ui32 AE_CALL getMipLevels() const = 0;
	};


	class AE_DLL ITextureResource : public ITextureViewBase {
	public:
		ITextureResource(IGraphicsModule& graphics) : ITextureViewBase(graphics) {}
		virtual ~ITextureResource() {}

		virtual TextureType AE_CALL getType() const = 0;
		virtual const void* AE_CALL getNativeResource() const = 0;
		virtual ui16 AE_CALL getPerPixelByteSize() const = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap(ui32 arraySlice, ui32 mipSlice) = 0;
		virtual ui32 AE_CALL read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen) = 0;
		virtual ui32 AE_CALL write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) = 0;
	};


	class AE_DLL ITexture1DResource : public ITextureResource {
	public:
		ITexture1DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture1DResource() {}

		virtual bool AE_CALL create(ui32 width, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box1ui32& range, const void* data) = 0;
		virtual bool AE_CALL copyFrom(ui32 arraySlice, ui32 mipSlice, const Box1ui32& range, const IPixelBuffer* pixelBuffer) = 0;
	};


	class AE_DLL ITexture2DResource : public ITextureResource {
	public:
		ITexture2DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture2DResource() {}

		virtual bool AE_CALL create(const Vec2ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box2ui32& range, const void* data) = 0;
		virtual bool AE_CALL copyFrom(ui32 arraySlice, ui32 mipSlice, const Box2ui32& range, const IPixelBuffer* pixelBuffer) = 0;
	};


	class AE_DLL ITexture3DResource : public ITextureResource {
	public:
		ITexture3DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture3DResource() {}

		virtual bool AE_CALL create(const Vec3ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box3ui32& range, const void* data) = 0;
		virtual bool AE_CALL copyFrom(ui32 arraySlice, ui32 mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) = 0;
	};


	class AE_DLL ITextureView : public ITextureViewBase {
	public:
		ITextureView(IGraphicsModule& graphics) : ITextureViewBase(graphics) {}
		virtual ~ITextureView() {}

		//virtual TextureType AE_CALL getType() const = 0;
		virtual ITextureResource* AE_CALL getResource() const = 0;
		virtual bool AE_CALL create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels, ui32 arrayBegin, ui32 arraySize) = 0;
	};


	enum class ProgramStage : ui8 {
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
		virtual bool AE_CALL use() = 0;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, ui32 count = (std::numeric_limits<ui32>::max)(), ui32 offset = 0) = 0;
	};


	struct AE_DLL GraphicsDeviceFeatures {
		bool supportSampler;
		bool supportTextureView;
		bool supportPixelBuffer;
		bool supportConstantBuffer;
		bool supportPersisientMap;
	};


	class AE_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ModuleType AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual const std::string& AE_CALL getVersion() const = 0;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const = 0;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() = 0;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() = 0;
		virtual IPixelBuffer* AE_CALL createPixelBuffer() = 0;
		virtual IProgram* AE_CALL createProgram() = 0;
		virtual ISampler* AE_CALL createSampler() = 0;
		virtual ITexture1DResource* AE_CALL createTexture1DResource() = 0;
		virtual ITexture2DResource* AE_CALL createTexture2DResource() = 0;
		virtual ITexture3DResource* AE_CALL createTexture3DResource() = 0;
		virtual ITextureView* AE_CALL createTextureView() = 0;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}