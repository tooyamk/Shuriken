#pragma once

#include "modules/IModule.h"

namespace aurora {
	template<typename T> class Rect;
}

namespace aurora::modules {
	class IGraphicsModule;

	class AE_DLL IGraphicsObject : public Ref {
	public:
		virtual ~IGraphicsObject();

	protected:
		IGraphicsObject(IGraphicsModule& graphics);

		IGraphicsModule* _graphics;
	};


	class AE_DLL IGraphicsIndexBuffer : public IGraphicsObject {
	public:
		IGraphicsIndexBuffer(IGraphicsModule& graphics);
		virtual ~IGraphicsIndexBuffer();
	};


	class AE_DLL IGraphicsProgram : public IGraphicsObject {
	public:
		IGraphicsProgram(IGraphicsModule& graphics);
		virtual ~IGraphicsProgram();

		virtual bool AE_CALL upload(const i8* vert, const i8* frag) = 0;
		virtual void AE_CALL use() = 0;
	};


	class AE_DLL IGraphicsVertexBuffer : public IGraphicsObject {
	public:
		IGraphicsVertexBuffer(IGraphicsModule& graphics);
		virtual ~IGraphicsVertexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) = 0;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual void AE_CALL flush() = 0;
		virtual void AE_CALL use() = 0;
	};


	class AE_DLL IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual bool AE_CALL createDevice() = 0;

		virtual IGraphicsIndexBuffer* AE_CALL createIndexBuffer() = 0;
		virtual IGraphicsProgram* AE_CALL createProgram() = 0;
		virtual IGraphicsVertexBuffer* AE_CALL createVertexBuffer() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}