#pragma once

#include "base/DynamicLib.h"
#include "modules/graphics/IGraphicsModule.h"
#include "modules/IInputModule.h"

namespace aurora::modules {
	template<typename RetType>
	class AE_TEMPLATE_DLL ModuleLoader : public Ref {
	public:
		using CreateModuleFn = RetType*(*)(const Args*);

		ModuleLoader() : _createFn(nullptr) {}
		virtual ~ModuleLoader() {}

		bool AE_CALL load(const i8* path) {
			if (_lib.isLoaded()) _lib.free();
			if (_lib.load(path)) {
				_createFn = (CreateModuleFn)_lib.getSymbolAddress(AE_TO_STRING(AE_CREATE_MODULE_FN_NAME));
				return _createFn;
			} else {
				_createFn = nullptr;
				return false;
			}
			return false;
		}

		void AE_CALL free() {
			_lib.free();
		}

		RetType* AE_CALL create(const Args* args) const {
			if (_createFn && _lib.isLoaded()) {
				return (RetType*)_createFn(args);
			} else {
				return nullptr;
			}
		}

	protected:
		DynamicLib _lib;
		CreateModuleFn _createFn;
	};

	using GraphicsModuleLoader = ModuleLoader<graphics::IGraphicsModule>;
	using InputModuleLoader = ModuleLoader<IInputModule>;
}