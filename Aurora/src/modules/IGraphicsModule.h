#pragma once

#include "modules/IModule.h"
#include "base/ByteArray.h"
#include "math/Box.h"
#include <unordered_map>

namespace aurora::modules::graphics {
	class IGraphicsModule;
	class ITexture1DView;
	class ITexture2DView;
	class ITexture3DView;
	
	class AE_DLL GraphicsAdapter {
	public:
		GraphicsAdapter();

		ui32 vendorId;
		ui32 deviceId;
		ui64 dedicatedSystemMemory;
		ui64 dedicatedVideoMemory;
		ui64 sharedSystemMemory;
		std::string description;

		static void AE_CALL query(std::vector<GraphicsAdapter>& dst);
		static GraphicsAdapter* AE_CALL autoChoose(std::vector<GraphicsAdapter>& adapters);
		static void AE_CALL autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<ui32>& dst);

	private:
		static f64 AE_CALL _calcScore(const GraphicsAdapter& adapter);
	};


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


	enum class Usage : ui32 {
		NONE = 0,
		CPU_READ = 1,
		CPU_WRITE = 1 << 1,
		GPU_WRITE = 1 << 2,

		CPU_WRITE_DISCARD = 1 << 3,
		CPU_WRITE_NO_OVERWRITE = 1 << 4,

		CPU_READ_WRITE = CPU_READ | CPU_WRITE,
		CPU_GPU_WRITE = CPU_WRITE | GPU_WRITE
	};
	AE_DEFINE_ENUM_BIT_OPERATIION(Usage);


	class AE_DLL IBuffer : public IObject {
	public:
		IBuffer(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IBuffer() {}

		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) = 0;
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap() = 0;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) = 0;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual i32 AE_CALL update(ui32 offset, const void* data, ui32 length) = 0;
		virtual void AE_CALL flush() = 0;
	};


	enum class VertexSize : ui8 {
		ONE,
		TWO,
		THREE,
		FOUR
	};


	enum class VertexType : ui8 {
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

		virtual void AE_CALL setFormat(VertexSize size, VertexType type) = 0;
	};


	class AE_DLL VertexBufferFactory {
	public:
		~VertexBufferFactory();

		IVertexBuffer* AE_CALL get(const std::string& name) const;
		void AE_CALL add(const std::string& name, IVertexBuffer* buffer);
		void AE_CALL remove(const std::string& name);
		void AE_CALL clear();

	private:
		std::unordered_map<std::string, IVertexBuffer*> _buffers;
	};


	enum class IndexType : ui8 {
		UI8,
		UI16,
		UI32
	};


	class AE_DLL IIndexBuffer : public IBuffer {
	public:
		IIndexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IIndexBuffer() {}

		virtual void AE_CALL setFormat(IndexType type) = 0;
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

		virtual void AE_CALL setMipLOD(f32 min, f32 max, f32 bias) = 0;
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
		virtual Usage AE_CALL getUsage() const = 0;
		virtual Usage AE_CALL map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) = 0;
		virtual void AE_CALL unmap(ui32 arraySlice, ui32 mipSlice) = 0;
		virtual i32 AE_CALL read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) = 0;
		virtual i32 AE_CALL write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) = 0;
	};


	class AE_DLL ITexture1DResource : public ITextureResource {
	public:
		ITexture1DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture1DResource() {}

		virtual bool AE_CALL create(ui32 width, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box1ui32& range, const void* data) = 0;
	};


	class AE_DLL ITexture2DResource : public ITextureResource {
	public:
		ITexture2DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture2DResource() {}

		virtual bool AE_CALL create(const Vec2ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box2ui32& range, const void* data) = 0;
	};


	class AE_DLL ITexture3DResource : public ITextureResource {
	public:
		ITexture3DResource(IGraphicsModule& graphics) : ITextureResource(graphics) {}
		virtual ~ITexture3DResource() {}

		virtual bool AE_CALL create(const Vec3ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) = 0;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box3ui32& range, const void* data) = 0;
	};


	class AE_DLL ITextureView : public ITextureViewBase {
	public:
		ITextureView(IGraphicsModule& graphics) : ITextureViewBase(graphics) {}
		virtual ~ITextureView() {}

		//virtual TextureType AE_CALL getType() const = 0;
		virtual ITextureResource* AE_CALL getResource() const = 0;
		virtual bool AE_CALL create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels, ui32 arrayBegin, ui32 arraySize) = 0;
	};


	class AE_DLL IConstantBuffer : public IBuffer {
	public:
		IConstantBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {}
		virtual ~IConstantBuffer() {}
	};


	enum class ShaderParameterUsage : ui8 {
		AUTO,
		SHARE,
		EXCLUSIVE
	};


	enum class ShaderParameterType : ui8 {
		OTHER,
		SAMPLER,
		TEXTURE
	};


	class AE_DLL ShaderParameter : public Ref {
	public:
		ShaderParameter(ShaderParameterUsage usage = ShaderParameterUsage::AUTO);
		~ShaderParameter();

		void AE_CALL releaseExclusiveBuffers();

		inline ShaderParameterUsage AE_CALL getUsage() const {
			return _usage;
		}

		void AE_CALL setUsage(ShaderParameterUsage usage);

		inline ShaderParameterType AE_CALL getType() const {
			return _type;
		}

		inline const void* AE_CALL getData() const {
			return _storageType == StorageType::DEFAULT ? &_data : _data.externalData;
		}

		inline ui16 AE_CALL getPerElementSize() const {
			return _perElementSize;
		}

		inline ui32 AE_CALL getSize() const {
			return _size;
		}

		inline ui32 AE_CALL getUpdateId() const {
			return _updateId;
		}

		inline ShaderParameter& AE_CALL setUpdated() {
			++_updateId;
			return *this;
		}

		template<typename T>
		inline ShaderParameter& AE_CALL set(Math::NumberType<T> value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::OTHER, true);
		}
		inline ShaderParameter& AE_CALL set(const ISampler* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::SAMPLER, false);
		}
		inline ShaderParameter& AE_CALL set(const ITextureViewBase* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::TEXTURE, false);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec2<T>& value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::OTHER, true);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec3<T>& value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::OTHER, true);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec4<T>& value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::OTHER,  true);
		}
		ShaderParameter& AE_CALL set(const void* data, ui32 size, ui16 perElementSize, ShaderParameterType type, bool copy);

		void AE_CALL clear();

	ae_internal_public:
		using EXCLUSIVE_FN = void(*)(void* callTarget, const ShaderParameter& param);
		void AE_CALL __setExclusive(void* callTarget, EXCLUSIVE_FN callback);
		void AE_CALL __releaseExclusive(void* callTarget, EXCLUSIVE_FN callback);

	private:
		enum class StorageType : ui8 {
			DEFAULT,
			INTERNAL,
			EXTERNAL
		};


		ShaderParameterUsage _usage;
		ShaderParameterType _type;
		StorageType _storageType;
		ui16 _perElementSize;
		ui32 _updateId;
		ui32 _size;
		union Data {
			i8 data[sizeof(f32) << 2];

			struct {
				i8* internalData;
				ui32 internalSize;
			};
			
			struct {
				bool externalRef;
				const void* externalData;
			};
		} _data;
		static const ui32 DEFAULT_DATA_SIZE = sizeof(Data);
		
		ui32 _exclusiveRc;
		void* _exclusiveFnTarget;
		EXCLUSIVE_FN _exclusiveFn;
	};


	class AE_DLL ShaderParameterFactory {
	public:
		~ShaderParameterFactory();

		ShaderParameter* AE_CALL get(const std::string& name) const;
		ShaderParameter* AE_CALL add(const std::string& name, ShaderParameter* parameter);
		void AE_CALL remove(const std::string& name);
		void AE_CALL clear();

	private:
		std::unordered_map<std::string, ShaderParameter*> _parameters;
	};


	enum class ProgramLanguage : ui8 {
		UNKNOWN,
		HLSL,
		DXIL,
		SPIRV,
		GLSL,
		GSSL,
		MSL
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


	class AE_DLL ProgramSource {
	public:
		ProgramSource();
		ProgramSource(ProgramSource&& value);

		ProgramLanguage language;
		ProgramStage stage;
		std::string version;
		std::string entryPoint;
		ByteArray data;

		ProgramSource& operator=(ProgramSource&& value);

		bool AE_CALL isValid() const;

		inline static std::string AE_CALL toHLSLShaderModel(const ProgramSource& source) {
			return toHLSLShaderModel(source.stage, source.version);
		}

		static std::string AE_CALL toHLSLShaderModel(ProgramStage stage, const std::string& version);

		inline static std::string AE_CALL getEntryPoint(const ProgramSource& source) {
			return getEntryPoint(source.entryPoint);
		}
		inline static std::string AE_CALL getEntryPoint(const std::string& entryPoint) {
			return entryPoint.empty() ? "main" : entryPoint;
		}
	};


	class AE_DLL IProgramSourceTranslator : public IModule {
	public:
		virtual ModuleType AE_CALL getType() const override { return ModuleType::UNKNOWN; }
		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string& targetVersion) = 0;
	};


	class AE_DLL IProgram : public IObject {
	public:
		IProgram(IGraphicsModule& graphics) : IObject(graphics) {}
		virtual ~IProgram() {}

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) = 0;
		virtual bool AE_CALL use() = 0;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, ui32 count = 0xFFFFFFFFui32, ui32 offset = 0) = 0;
	};


	struct AE_DLL GraphicsDeviceFeatures {
		bool supportSampler;
		bool supportTextureView;
		bool supportConstantBuffer;
	};


	class AE_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ModuleType AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) = 0;

		virtual const std::string& AE_CALL getModuleVersion() const = 0;
		virtual const std::string& AE_CALL getDeviceVersion() const = 0;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const = 0;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() = 0;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() = 0;
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