#pragma once

#include "base/Ref.h"

namespace aurora::modules::graphics {
	class GraphicsModule;

	class AE_DLL GObject : public Ref {
	public:
		GObject(GraphicsModule& graphics);
		virtual ~GObject();

	protected:
		GraphicsModule* _graphics;
	};
}