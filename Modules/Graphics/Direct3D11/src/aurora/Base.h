#pragma once

#include "aurora/modules/graphics/IGraphicsModule.h"
#include "aurora/IApplication.h"

#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>


namespace aurora::modules::graphics::d3d11 {
	class DXObjGuard {
	public:
		~DXObjGuard() {
			clear();
		}

		inline void add(IUnknown* obj) {
			if (obj) _objs.emplace_back(obj);
		}
		inline void clear() {
			for (auto obj : _objs) obj->Release();
			_objs.clear();
		}

	private:
		std::vector<IUnknown*> _objs;
	};
}