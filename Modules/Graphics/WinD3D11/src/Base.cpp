#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	DXObjGuard::~DXObjGuard() {
		clear();
	}

	void DXObjGuard::add(IUnknown* obj) {
		if (obj) _objs.emplace_back(obj);
	}

	void DXObjGuard::clear() {
		for (auto obj : _objs) obj->Release();
		_objs.clear();
	}
}