#pragma once

#include "srk/Global.h"

namespace srk {
	class SRK_CORE_DLL DynamicLibraryLoader {
	public:
		DynamicLibraryLoader();
		DynamicLibraryLoader(const DynamicLibraryLoader& other) = delete;
		DynamicLibraryLoader(DynamicLibraryLoader&& other) noexcept;
		virtual ~DynamicLibraryLoader();

		DynamicLibraryLoader& SRK_CALL operator=(const DynamicLibraryLoader&) = delete;

		inline DynamicLibraryLoader& SRK_CALL operator=(DynamicLibraryLoader&& other) noexcept {
			release();
			_lib = other._lib;
			other._lib = nullptr;
			return *this;
		}

		inline bool SRK_CALL isLoaded() const { return _lib; }

		template<typename T>
		requires ConvertibleString8Data<std::remove_cvref_t<T>>
		inline bool SRK_CALL load(T&& path) {
			return _load((const std::string_view&)(const ConvertToString8ViewType<std::remove_cvref_t<T>>&)(std::forward<T>(path)));
		}
		void SRK_CALL release();
		void* SRK_CALL getSymbolAddress(const std::string_view& name) const;

	private:
		void* _lib;

		bool SRK_CALL _load(const std::string_view& path);
	};
}