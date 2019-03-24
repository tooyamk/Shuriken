#pragma once

#include "base/Ref.h"

namespace aurora {
	template<typename T> class Rect;
}

namespace aurora::modules::graphics {
	class Program;
	class VertexBuffer;

	class AE_DLL GraphicsModule : public Ref {
	public:
		virtual ~GraphicsModule();

		virtual bool AE_CALL createView(void* style, const i8* windowTitle, const aurora::Rect<i32>& windowedRect, bool fullscreen) = 0;
		virtual bool AE_CALL isWindowed() const = 0;
		virtual void AE_CALL toggleFullscreen() = 0;
		virtual void AE_CALL getWindowedRect(aurora::Rect<i32>& dst) const = 0;
		virtual void AE_CALL setWindowedRect(const aurora::Rect<i32>& rect) = 0;

		virtual VertexBuffer* AE_CALL createVertexBuffer() = 0;
		virtual Program* AE_CALL createProgram() = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}