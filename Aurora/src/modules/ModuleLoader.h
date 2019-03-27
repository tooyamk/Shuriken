#pragma once

#include "base/DynamicLib.h"
#include "modules/GraphicsModule.h"
#include "modules/InputModule.h"

namespace aurora::events {
	template<typename EvtType> class IEventDispatcherAllocator;
}

namespace aurora::modules {
	template<typename CreateFn>
	class AE_TEMPLATE_DLL ModuleLoader : public Ref {
	public:
		virtual ~ModuleLoader() {}

		bool AE_CALL load(const i8* path) {
			if (_lib.isLoaded()) _lib.free();
			if (_lib.load(path)) {
				_createFn = (CreateFn)_lib.getSymbolAddress(AE_TO_STRING(AE_CREATE_MODULE_FN_NAME));
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

	protected:
		ModuleLoader() : _createFn(nullptr) {}

		DynamicLib _lib;
		CreateFn _createFn;
	};


	class GraphicsModuleLoader : public ModuleLoader<GraphicsModule::CREATE_MODULE_FN> {
	public:
		GraphicsModule* AE_CALL create() const {
			if (_createFn && _lib.isLoaded()) {
				return (GraphicsModule*)_createFn();
			} else {
				return nullptr;
			}
		}
	};


	class InputModuleLoader : public ModuleLoader<InputModule::CREATE_MODULE_FN> {
	public:
		InputModule* AE_CALL create(InputModule::EVENT_DISPATCHER_ALLOCATOR allocator) const {
			if (_createFn && _lib.isLoaded()) {
				return (InputModule*)_createFn(allocator);
			} else {
				return nullptr;
			}
		}
	};
}