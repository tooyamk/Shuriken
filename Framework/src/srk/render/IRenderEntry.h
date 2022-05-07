#pragma once

#include "srk/Intrusive.h"

namespace srk::render {
	class IRenderEntry : public Ref {
	public:
		virtual ~IRenderEntry() {}
	};
}