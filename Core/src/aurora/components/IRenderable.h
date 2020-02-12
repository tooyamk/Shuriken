#pragma once

#include "aurora/components/IComponent.h"
#include "aurora/Material.h"
#include <vector>

namespace aurora::components {
	class AE_DLL IRenderable : public IComponent {
	public:
		IRenderable();

		std::vector<RefPtr<Material>> materials;

	protected:
		AE_RTTI_DECLARE_DERIVED(IComponent);
	};
}