#pragma once

#include "modules/graphics/GObject.h"

namespace aurora::modules::graphics {
	class AE_DLL VertexBuffer : public GObject {
	public:
		virtual ~VertexBuffer();

		virtual bool AE_CALL stroage(ui32 size, const void* data = nullptr) = 0;
		virtual void AE_CALL write(ui32 offset, const void* data, ui32 length) = 0;
		virtual void AE_CALL flush() = 0;
		virtual void AE_CALL use() = 0;

	protected:
		VertexBuffer(GraphicsModule& graphics);
	};
}