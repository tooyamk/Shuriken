#pragma once

#include "srk/Global.h"

namespace srk {
	class SRK_CORE_DLL DynamicLib {
	public:
		DynamicLib();
		virtual ~DynamicLib();

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