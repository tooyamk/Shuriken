#pragma once

#include "srk/DynamicLibraryLoader.h"
#include "srk/modules/graphics/GraphicsModule.h"
#include "srk/modules/inputs/InputModule.h"
#include "srk/modules/windows/WindowModule.h"

namespace srk::modules {
	template<IntrusivePtrOperableObject RetType, typename... Args>
	class ModuleLoader : public Ref {
	public:
		using CreateModuleFn = RetType*(*)(Ref* loader, Args...);

		ModuleLoader() : _createFn(nullptr) {}
		virtual ~ModuleLoader() {}

		template<typename T>
		requires ConvertibleString8Data<std::remove_cvref_t<T>>
		bool SRK_CALL load(T&& path) {
			if (_lib.load(std::forward<T>(path))) {
				_createFn = (CreateModuleFn)_lib.getSymbolAddress(SRK_TO_STRING(SRK_CREATE_MODULE_FN_NAME));
				if (_createFn) {
					return true;
				} else {
					free();
					return false;
				}
			} else {
				_createFn = nullptr;
				return false;
			}
		}

		inline void SRK_CALL free() {
			_lib.release();
			_createFn = nullptr;
		}

		IntrusivePtr<RetType> SRK_CALL create(Args... args) {
			if (!_createFn) return nullptr;
			return (RetType*)_createFn(this, args...);
		}

	protected:
		DynamicLibraryLoader _lib;
		CreateModuleFn _createFn;
	};

	using GraphicsModuleLoader = ModuleLoader<graphics::IGraphicsModule, const graphics::CreateGrahpicsModuleDescriptor&>;
	using InputModuleLoader = ModuleLoader<inputs::IInputModule, const inputs::CreateInputModuleDescriptor&>;
	using WindowModuleLoader = ModuleLoader<windows::IWindowModule>;
}