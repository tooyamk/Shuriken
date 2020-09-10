#pragma once

#include "aurora/DynamicLib.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include "aurora/modules/inputs/IInputModule.h"

namespace aurora::modules {
	template<typename RetType>
	class ModuleLoader : public Ref {
	public:
		using CreateModuleFn = RetType*(*)(Ref* loader, const Args*);

		ModuleLoader() : _createFn(nullptr) {}
		virtual ~ModuleLoader() {}

		bool AE_CALL load(const std::string_view& path) {
			if (_lib.isLoaded()) _lib.release();
			if (_lib.load(path)) {
				_createFn = (CreateModuleFn)_lib.getSymbolAddress(AE_TO_STRING(AE_CREATE_MODULE_FN_NAME));
				return _createFn;
			} else {
				_createFn = nullptr;
				return false;
			}
			return false;
		}

		inline void AE_CALL free() {
			_lib.release();
		}

		RetType* AE_CALL create(const Args* args) {
			if (_createFn && _lib.isLoaded()) {
				return (RetType*)_createFn(this, args);
			} else {
				return nullptr;
			}
		}

	protected:
		DynamicLib _lib;
		CreateModuleFn _createFn;
	};

	using GraphicsModuleLoader = ModuleLoader<graphics::IGraphicsModule>;
	using InputModuleLoader = ModuleLoader<inputs::IInputModule>;
}