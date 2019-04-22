#pragma once

#include "modules/IModule.h"
#include "base/ByteArray.h"

namespace aurora {
	template<typename T> class Rect;
	class Vector2;
	class Vector3;
	class Vector4;
}

namespace aurora::modules::graphics {
	class IGraphicsModule;

	
	class AE_DLL GraphicsAdapter {
	public:
		GraphicsAdapter();

		ui32 vendorId;
		ui32 deviceId;
		ui64 dedicatedSystemMemory;
		ui64 dedicatedVideoMemory;
		ui64 sharedSystemMemory;
		std::string description;

		static void query(std::vector<GraphicsAdapter>& dst);
		static GraphicsAdapter* autoChoose(std::vector<GraphicsAdapter>& adapters);
		static void autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<ui32>& dst);

	private:
		static f64 _calcScore(const GraphicsAdapter& adapter);
	};


	class AE_DLL IObject : public Ref {
	public:
		virtual ~IObject();

	protected:
		IObject(IGraphicsModule& graphics);

		IGraphicsModule* _graphics;
	};


	class AE_DLL BufferUsage {
	public:
		AE_DECLA_CANNOT_INSTANTIATE(BufferUsage);

		static const ui32 CPU_READ = 1;
		static const ui32 CPU_WRITE = 1 << 1;
		static const ui32 GPU_WRITE = 1 << 2;

		static const ui32 CPU_WRITE_DISCARD = 1 << 3;
		static const ui32 CPU_WRITE_NO_OVERWRITE = 1 << 4;

		static const ui32 CPU_READ_WRITE = CPU_READ | CPU_WRITE;
		static const ui32 CPU_GPU_WRITE = CPU_WRITE | GPU_WRITE;
	};


	class AE_DLL IBuffer : public IObject {
	public:
		IBuffer(IGraphicsModule& graphics);
		virtual ~IBuffer();

		virtual bool AE_CALL allocate(ui32 size, ui32 bufferUsage, const void* data = nullptr) = 0;
		virtual ui32 AE_CALL map(ui32 mapUsage) = 0;
		virtual void AE_CALL unmap() = 0;
		virtual i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) = 0;
		virtual i32 AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
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
		IVertexBuffer(IGraphicsModule& graphics);
		virtual ~IVertexBuffer();

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
		IIndexBuffer(IGraphicsModule& graphics);
		virtual ~IIndexBuffer();

		virtual void AE_CALL setFormat(IndexType type) = 0;
	};


	enum class SamplerFilterMode : ui8 {
		NEAREST,
		LINEAR,
		ANISOTROPIC
	};


	enum class SamplerFilterOperation : ui8 {
		NORMAL,
		COMPARISON,
		MINIMUM,
		MAXIMUM
	};


	enum class SamplerAddressMode : ui8 {

	};


	struct AE_DLL SamplerFilter {
		SamplerFilter();
		SamplerFilterOperation operation;
		SamplerFilterMode minification;
		SamplerFilterMode magnification;
		SamplerFilterMode mipmap;
	};


	class AE_DLL ISampler : public IObject {
	public:
		ISampler(IGraphicsModule& graphics);
		virtual ~ISampler();

		virtual void AE_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) = 0;
		virtual void AE_CALL setFilter(const SamplerFilter& filter) = 0;
	};


	enum class TextureType : ui8 {
		TEX1D,
		TEX2D,
		TEX3D
	};


	enum class TextureFormat : ui8 {
		R8G8B8A8
	};


	class AE_DLL ITexture : public IObject {
	public:
		ITexture(IGraphicsModule& graphics);
		virtual ~ITexture();

		virtual TextureType AE_CALL getType() const = 0;
	};


	class AE_DLL ITexture2D : public ITexture {
	public:
		ITexture2D(IGraphicsModule& graphics);
		virtual ~ITexture2D();

		virtual bool AE_CALL allocate(ui32 width, ui32 height, TextureFormat format, ui32 bufferUsage, const void* data = nullptr) = 0;
	};


	class AE_DLL IConstantBuffer : public IBuffer {
	public:
		IConstantBuffer(IGraphicsModule& graphics);
		virtual ~IConstantBuffer();
	};


	enum class ShaderParameterUsage : ui8 {
		AUTO,
		SHARE,
		EXCLUSIVE
	};


	class AE_DLL ShaderParameter : public Ref {
	public:
		ShaderParameter(ShaderParameterUsage usage = ShaderParameterUsage::AUTO);
		~ShaderParameter();

		void releaseExclusiveBuffers();

		inline ShaderParameterUsage AE_CALL getUsage() const {
			return _usage;
		}

		void setUsage(ShaderParameterUsage usage);

		inline const void* AE_CALL getData() const {
			return _type == Type::DEFAULT ? &_data : _data.externalData;
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

		ShaderParameter& AE_CALL set(f32 value);
		ShaderParameter& AE_CALL set(const ISampler* value);
		ShaderParameter& AE_CALL set(const ITexture* value);
		ShaderParameter& AE_CALL set(const Vector2& value);
		ShaderParameter& AE_CALL set(const Vector3& value);
		ShaderParameter& AE_CALL set(const Vector4& value);
		ShaderParameter& AE_CALL set(const void* data, ui32 size, ui16 perElementSize, bool copy);

	ae_internal_public:
		using EXCLUSIVE_FN = void(*)(void* callTarget, const ShaderParameter& param);
		void AE_CALL __setExclusive(void* callTarget, EXCLUSIVE_FN callback);
		void AE_CALL __releaseExclusive(void* callTarget, EXCLUSIVE_FN callback);

	private:
		enum class Type : ui8 {
			DEFAULT,
			INTERNAL,
			EXTERNAL
		};


		ShaderParameterUsage _usage;
		Type _type;
		ui16 _perElementSize;
		ui32 _updateId;
		ui32 _size;
		union Data {
			i8 data[sizeof(f32) << 2];

			struct {
				i8* internalData;
				ui32 internalSize;
			};
			
			const void* externalData;
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
		ShaderParameter* AE_CALL add(const std::string& name, ShaderParameter* buffer);
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

		bool isValid() const;

		inline static std::string toHLSLShaderModel(const ProgramSource& source) {
			return toHLSLShaderModel(source.stage, source.version);
		}

		static std::string toHLSLShaderModel(ProgramStage stage, const std::string& version);

		inline static std::string getEntryPoint(const ProgramSource& source) {
			return getEntryPoint(source.entryPoint);
		}

		inline static std::string getEntryPoint(const std::string& entryPoint) {
			return entryPoint.empty() ? "main" : entryPoint;
		}
	};


	class AE_DLL IProgramSourceTranslator : public IModule {
	public:
		virtual ui32 AE_CALL getType() const { return 0; }
		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string& targetVersion) = 0;
	};


	class AE_DLL IProgram : public IObject {
	public:
		IProgram(IGraphicsModule& graphics);
		virtual ~IProgram();

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) = 0;
		virtual bool AE_CALL use() = 0;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, ui32 count = 0xFFFFFFFFui32, ui32 offset = 0) = 0;
	};


	class AE_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) = 0;

		virtual IConstantBuffer* AE_CALL createConstantBuffer() = 0;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() = 0;
		virtual IProgram* AE_CALL createProgram() = 0;
		virtual ISampler* AE_CALL createSampler() = 0;
		virtual ITexture2D* AE_CALL createTexture2D() = 0;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}