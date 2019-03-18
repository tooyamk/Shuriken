#pragma once

#include "base/Aurora.h"

namespace aurora {
	template<typename T> class Rect;
}

namespace aurora::module::graphics{
	class GraphicsModule {
	public:
		virtual bool AE_CALL createView(void* style, const i8* windowTitle, const aurora::Rect<i32>& rect, bool fullscreen) = 0;
		virtual bool AE_CALL isWindowed() const = 0;
		virtual void AE_CALL toggleFullscreen() = 0;
		virtual void AE_CALL getViewRect(aurora::Rect<i32>& dst) const = 0;
		virtual void AE_CALL setViewRect(const aurora::Rect<i32>& rect) = 0;

		virtual void AE_CALL beginRender() = 0;
		virtual void AE_CALL endRender() = 0;
		virtual void AE_CALL present() = 0;

		virtual void AE_CALL clear() = 0;
	};
}