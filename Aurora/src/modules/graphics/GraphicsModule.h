#pragma once

#include "base/Ref.h"

namespace aurora {
	class Application;
	template<typename T> class Rect;
}

namespace aurora::modules::graphics {
	class Program;
	class VertexBuffer;

	class AE_DLL GraphicsModule : public Ref {
	public:
		virtual ~GraphicsModule();

		virtual bool AE_CALL createDevice(Application* app) = 0;

		virtual VertexBuffer* AE_CALL createVertexBuffer() = 0;
		virtual Program* AE_CALL createProgram() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}