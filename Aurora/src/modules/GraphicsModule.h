#pragma once

#include "modules/Module.h"

namespace aurora {
	template<typename T> class Rect;
}

namespace aurora::modules {
	class AE_DLL GraphicsModule : public Module {
	public:
		class AE_DLL Object : public Ref {
		public:
			virtual ~Object();

		protected:
			Object(GraphicsModule& graphics);

			GraphicsModule* _graphics;
		};


		class AE_DLL Program : public Object {
		public:
			virtual ~Program();

			virtual void AE_CALL use() = 0;

		protected:
			Program(GraphicsModule& graphics);
		};


		class AE_DLL VertexBuffer : public Object {
		public:
			virtual ~VertexBuffer();

			virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) = 0;
			virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
			virtual void AE_CALL flush() = 0;
			virtual void AE_CALL use() = 0;

		protected:
			VertexBuffer(GraphicsModule& graphics);
		};


		virtual ~GraphicsModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::GRAPHICS;
		}

		virtual bool AE_CALL createDevice() = 0;

		virtual VertexBuffer* AE_CALL createVertexBuffer() = 0;
		virtual Program* AE_CALL createProgram() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}