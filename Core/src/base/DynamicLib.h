#pragma once

#include "base/Global.h"

namespace aurora {
	class AE_DLL DynamicLib {
	public:
		DynamicLib();
		virtual ~DynamicLib();

		inline bool AE_CALL isLoaded() const { return _lib; }
		bool AE_CALL load(const std::string_view& path);
		void AE_CALL release();
		void* AE_CALL getSymbolAddress(const std::string_view& name) const;

	private:
		void* _lib;
	};
}