#pragma once

#include "modules/graphics/IGraphicsModule.h"
#include "base/Application.h"

#include <d3d11_4.h>
#pragma comment(lib, "d3d11.lib")

#include <d3dcompiler.h>
#pragma  comment(lib,"d3dcompiler.lib")

#include <dxgi1_6.h>
#pragma  comment(lib,"dxgi.lib")

#pragma  comment(lib,"dxguid.lib")

namespace aurora::modules::graphics::win_d3d11 {
	class DXObjGuard {
	public:
		~DXObjGuard();

		void add(IUnknown* obj);
		void clear();
	private:
		std::vector<IUnknown*> _objs;
	};
}