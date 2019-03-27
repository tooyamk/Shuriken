#pragma once

#include "base/Ref.h"

namespace aurora::modules {
	class AE_DLL Module : public Ref {
	public:
		virtual ~Module() {}
	};
}