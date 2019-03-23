#pragma once

#include "base/Ref.h"

namespace aurora::modules::graphics {
	class VertexBuffer : public Ref {
	public:
		virtual ~VertexBuffer() {}

		virtual bool stroage(ui32 size, const void* data = nullptr) = 0;
	};
}