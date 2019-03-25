#pragma once

#include "modules/graphics/GObject.h"

namespace aurora::modules::graphics {
	class GraphicsModule;

	class AE_DLL Program : public GObject {
	public:
		virtual ~Program();

		virtual void AE_CALL use() = 0;

	protected:
		Program(GraphicsModule& graphics);
	};
}