#pragma once

#include "base/LowLevel.h"
#include <atomic>

namespace aurora {
	class AE_TEMPLATE_DLL Ref {
	public:
		template<typename T>
		using RefType = std::enable_if_t<std::is_base_of_v<Ref, T>, T>;

		Ref() :
			_refCount(0) {
		}

		virtual ~Ref() {
		}

		inline void AE_CALL ref() {
			_refCount.fetch_add(1, std::memory_order_release);
		}
		inline void AE_CALL unref() {
			if (_refCount.fetch_sub(1) <= 1) delete this;
		}

		inline ui32 AE_CALL getReferenceCount() const {
			return _refCount.load(std::memory_order_acquire);
		}

		template<typename T>
		inline RefType<T>* ref() {
			ref();
			return (T*)this;
		}

		template<typename P1, typename P2>
		inline static void AE_CALL set(RefType<P1>*& ptr, RefType<P2>* target) {
			if (ptr != target) {
				if (target) target->ref();
				if (ptr) ptr->unref();
				ptr = target;
			}
		}

		template<typename P>
		inline static void AE_CALL setNull(RefType<P>*& ptr) {
			ptr->unref();
			ptr = nullptr;
		}

		template<typename P>
		inline static void AE_CALL checkSetNull(RefType<P>*& ptr) {
			if (ptr) setNull(ptr);
		}

	protected:
		std::atomic_uint32_t _refCount;
	};


	template<typename T>
	class AE_TEMPLATE_DLL RefPtr {
	public:
		RefPtr() :
			_target(nullptr) {
		}

		RefPtr(const RefPtr<T>& ptr) : RefPtr(ptr._target) {
		}

		RefPtr(RefPtr<T>&& ptr) :
			_target(ptr._target) {
			ptr._target = nullptr;
		}

		RefPtr(T* target) :
			_target(target ? target->ref<T>() : nullptr) {
		}

		~RefPtr() {
			reset();
		}

		RefPtr<T>& operator=(RefPtr<T>&& ptr) {
			auto target = ptr._target;
			if (_target != target) {
				if (_target) _target->unref();
				_target = target;
			}
			ptr._target = nullptr;
		}

		inline void operator=(T* target) {
			set(target);
		}

		inline void operator=(const RefPtr<T>& ptr) {
			set(ptr._target);
		}

		inline T* AE_CALL get() const {
			return _target;
		}

		inline void AE_CALL set(T* target) {
			if (_target != target) {
				if (target) target->ref();
				if (_target) _target->unref();
				_target = target;
			}
		}

		inline void AE_CALL reset() {
			if (_target) {
				_target->unref();
				_target = nullptr;
			}
		}

		inline bool AE_CALL isEmpty() const {
			return !_target;
		}

	private:
		Ref::RefType<T>* _target;
	};
}
