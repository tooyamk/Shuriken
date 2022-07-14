#pragma once

#include "srk/DynamicLibraryLoader.h"
#include "srk/modules/graphics/IGraphicsModule.h"
#include "srk/modules/inputs/IInputModule.h"

namespace srk::modules {
	template<IntrusivePtrOperableObject RetType>
	class ModuleLoader : public Ref {
	public:
		using CreateModuleFn = RetType*(*)(Ref* loader, const SerializableObject*);

		ModuleLoader() : _createFn(nullptr) {}
		virtual ~ModuleLoader() {}

		template<typename T>
		requires ConvertibleString8Data<std::remove_cvref_t<T>>
		bool SRK_CALL load(T&& path) {
			if (_lib.isLoaded()) _lib.release();
			if (_lib.load(std::forward<T>(path))) {
				_createFn = (CreateModuleFn)_lib.getSymbolAddress(SRK_TO_STRING(SRK_CREATE_MODULE_FN_NAME));
				return _createFn;
			} else {
				_createFn = nullptr;
				return false;
			}
			return false;
		}

		inline void SRK_CALL free() {
			_lib.release();
		}

		IntrusivePtr<RetType> SRK_CALL create(const SerializableObject* args) {
			if (_createFn && _lib.isLoaded()) {
				return (RetType*)_createFn(this, args);
			} else {
				return nullptr;
			}
		}

	protected:
		DynamicLibraryLoader _lib;
		CreateModuleFn _createFn;
	};

	using GraphicsModuleLoader = ModuleLoader<graphics::IGraphicsModule>;
	using InputModuleLoader = ModuleLoader<inputs::IInputModule>;
}