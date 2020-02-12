#pragma once

#include "aurora/Ref.h"

namespace aurora::renderers {
	class IRenderEntry : public Ref {
	public:
		virtual ~IRenderEntry() {}
	};
}