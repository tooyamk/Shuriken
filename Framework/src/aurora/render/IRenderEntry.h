#pragma once

#include "aurora/Intrusive.h"

namespace aurora::render {
	class IRenderEntry : public Ref {
	public:
		virtual ~IRenderEntry() {}
	};
}