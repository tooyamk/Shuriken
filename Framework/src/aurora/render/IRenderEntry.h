#pragma once

#include "aurora/Ref.h"

namespace aurora::render {
	class IRenderEntry : public Ref {
	public:
		virtual ~IRenderEntry() {}
	};
}