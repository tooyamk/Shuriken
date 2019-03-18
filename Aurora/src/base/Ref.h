#pragma once

#include "base/Aurora.h"
#include <atomic>

namespace aurora {
#define AE_SET_REF(__TARGET__, __NEW__) \
	do { \
		if (__NEW__ != nullptr) __NEW__->ref(); \
		if (__TARGET__ != nullptr) __TARGET__->unref(); \
		__TARGET__ = __NEW__; \
	} while(0) \

#define AE_FREE_REF(__TARGET__) \
	do { \
		if (__TARGET__ != nullptr) { \
			__TARGET__->unref(); \
			__TARGET__ = nullptr; \
		} \
	} while(0) \

	class AE_DLL Ref {
	public:
		Ref();
		virtual ~Ref();

		inline void AE_CALL ref() {
			++_refCount;
		}
		inline void AE_CALL unref() {
			if (--_refCount == 0) delete this;
		}

		inline ui32 AE_CALL getReferenceCount() const {
			return _refCount;
		}

	protected:
		std::atomic_uint32_t _refCount;
	};
}
