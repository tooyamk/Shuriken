#pragma once

#include "base/Global.h"

namespace aurora {
	class AE_DLL DynamicLib {
	public:
		DynamicLib();
		virtual ~DynamicLib();

		inline bool AE_CALL isLoaded() const { return _lib; }
		bool AE_CALL load(const char* path);
		void AE_CALL free();
		void* AE_CALL getSymbolAddress(const char* name) const;

	private:
		void* _lib;
	};
}