#pragma once

#include "base/LowLevel.h"
#include <atomic>

namespace aurora {
	class AE_TEMPLATE_DLL Ref {
	public:
		Ref() : _refCount(0) {}
		virtual ~Ref() {}

		inline void AE_CALL ref() {
			_refCount.fetch_add(1, std::memory_order_relaxed);
		}
		inline void AE_CALL unref() {
			if (_refCount.fetch_sub(1, std::memory_order_acquire) <= 1) delete this;
		}

		inline ui32 AE_CALL getReferenceCount() const {
			return _refCount.load(std::memory_order_relaxed);
		}

		template<typename T>
		inline std::enable_if_t<std::is_base_of_v<Ref, T>, T> * ref() {
			ref();
			return (T*)this;
		}

		template<typename P1, typename P2>
		inline static void AE_CALL set(P1& ptr, P2 target) {
			if (ptr != target) {
				if (target) target->ref();
				if (ptr) ptr->unref();
				ptr = target;
			}
		}

		template<typename P>
		inline static void AE_CALL setNull(P ptr) {
			ptr->unref();
			ptr = nullptr;
		}

		template<typename P>
		inline static void AE_CALL checkSetNull(P ptr) {
			if (ptr) setNull(ptr);
		}

	protected:
		std::atomic_uint32_t _refCount;
	};

	
	template<typename T>
	class AE_TEMPLATE_DLL RefGuard {
	public:
		RefGuard(T* target) :
			_target(target) {
			_target->ref();
		}

		~RefGuard() {
			_target->unref();
		}

	private:
		T* _target;
	};
}
