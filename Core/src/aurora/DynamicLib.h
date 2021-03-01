#pragma once

#include "aurora/Global.h"

namespace aurora {
	class AE_CORE_DLL DynamicLib {
	public:
		DynamicLib();
		virtual ~DynamicLib();

		inline bool AE_CALL isLoaded() const { return _lib; }

		template<typename T, typename = std::enable_if_t<is_any_string8_type_v<T>>>
		inline bool AE_CALL load(T&& path) {
			return _load((const std::string_view&)string8_view_t<T>(std::forward<T>(path)));
		}
		void AE_CALL release();
		void* AE_CALL getSymbolAddress(const std::string_view& name) const;

	private:
		void* _lib;

		bool AE_CALL _load(const std::string_view& path);
	};
}