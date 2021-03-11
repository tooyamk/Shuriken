#pragma once

#include "aurora/DynamicLib.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include "aurora/modules/inputs/IInputModule.h"

namespace aurora::modules {
	template<typename RetType>
	class ModuleLoader : public Ref {
	public:
		using CreateModuleFn = RetType*(*)(Ref* loader, const SerializableObject*);

		ModuleLoader() : _createFn(nullptr) {}
		virtual ~ModuleLoader() {}

		template<typename T, typename = std::enable_if_t<is_convertible_string8_data_v<std::remove_cvref_t<T>>>>
		bool AE_CALL load(T&& path) {
			if (_lib.isLoaded()) _lib.release();
			if (_lib.load(std::forward<T>(path))) {
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

		IntrusivePtr<RetType> AE_CALL create(const SerializableObject* args) {
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