#pragma once

#include "base/DynamicLib.h"
#include "modules/GraphicsModule.h"
#include "modules/InputModule.h"

namespace aurora::events {
	template<typename EvtType> class IEventDispatcherAllocator;
}

namespace aurora::modules {
	template<typename EvtType, typename RetType>
	class AE_TEMPLATE_DLL ModuleLoader : public Ref {
	public:
		using CREATE_MODULE_FN = RetType*(*)(const ModuleCreateParams<EvtType>*);

		ModuleLoader() : _createFn(nullptr) {}
		virtual ~ModuleLoader() {}

		bool AE_CALL load(const i8* path) {
			if (_lib.isLoaded()) _lib.free();
			if (_lib.load(path)) {
				_createFn = (CREATE_MODULE_FN)_lib.getSymbolAddress(AE_TO_STRING(AE_CREATE_MODULE_FN_NAME));
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

		RetType* AE_CALL create(const ModuleCreateParams<EvtType>* params) const {
			if (_createFn && _lib.isLoaded()) {
				return (RetType*)_createFn(params);
			} else {
				return nullptr;
			}
		}

		RetType* AE_CALL create(const ModuleCreateParams<EvtType>& params) const {
			return create(&params);
		}

	protected:
		DynamicLib _lib;
		CREATE_MODULE_FN _createFn;
	};

	using GraphicsModuleLoader = ModuleLoader<GraphicsModule::EVENT, GraphicsModule>;
	using InputModuleLoader = ModuleLoader<InputModule::EVENT, InputModule>;
}