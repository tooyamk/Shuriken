#pragma once

#include "modules/IModule.h"
#include "base/ByteArray.h"

namespace aurora {
	template<typename T> class Rect;
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


	enum class VertexSize {
		ONE,
		TWO,
		THREE,
		FOUR
	};


	enum class VertexType {
		I8,
		UI8,
		I16,
		UI16,
		I32,
		UI32,
		F32
	};


	class AE_DLL IVertexBuffer : public IObject {
	public:
		IVertexBuffer(IGraphicsModule& graphics);
		virtual ~IVertexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) = 0;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual void AE_CALL setFormat(VertexSize size, VertexType type) = 0;
		virtual void AE_CALL flush() = 0;
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


	enum class IndexType {
		UI8,
		UI16,
		UI32
	};


	class AE_DLL IIndexBuffer : public IObject {
	public:
		IIndexBuffer(IGraphicsModule& graphics);
		virtual ~IIndexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) = 0;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual void AE_CALL setFormat(IndexType type) = 0;
		virtual void AE_CALL flush() = 0;
	};


	class AE_DLL IConstantBuffer : public IObject {
	public:
		IConstantBuffer(IGraphicsModule& graphics);
		virtual ~IConstantBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) = 0;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual void AE_CALL flush() = 0;
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
		virtual void AE_CALL useVertexBuffers(const VertexBufferFactory& factory) = 0;
		virtual void AE_CALL draw(const IIndexBuffer& indexBuffer, ui32 count = 0xFFFFFFFFui32, ui32 offset = 0) = 0;
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
		virtual IVertexBuffer* AE_CALL createVertexBuffer() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}