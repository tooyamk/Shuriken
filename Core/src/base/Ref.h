#pragma once

#include "base/LowLevel.h"
#include <atomic>

namespace aurora {
	class AE_TEMPLATE_DLL Ref {
	public:
		template<typename T>
		//using RefType = std::enable_if_t<std::is_base_of_v<Ref, T>, T>;
		using RefType = T;

		Ref() :
			_refCount(0) {
		}

		virtual ~Ref() {
		}

		inline void AE_CALL ref() {
			_refCount.fetch_add(1, std::memory_order_release);
		}

		template<typename T>
		inline RefType<T>* ref() {
			ref();
			return (T*)this;
		}

		inline void AE_CALL unref() {
			if (_refCount.fetch_sub(1) <= 1) delete this;
		}

		inline ui32 AE_CALL getReferenceCount() const {
			return _refCount.load(std::memory_order_acquire);
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

		RefPtr(T& target) :
			_target(target.ref<T>()) {
		}

		~RefPtr() {
			reset();
		}

		inline RefPtr<T>& AE_CALL operator=(RefPtr<T>&& ptr) {
			if (_target) _target->unref();
			_target = ptr._target;
			ptr._target = nullptr;
			return *this;
		}

		inline void AE_CALL operator=(T* target) {
			set(target);
		}

		inline void AE_CALL operator=(T& target) {
			set(target);
		}

		inline void AE_CALL operator=(const RefPtr<T>& ptr) {
			set(ptr._target);
		}

		inline bool AE_CALL operator==(const T* target) const {
			return _target == target;
		}

		inline bool AE_CALL operator==(const T& target) const {
			return _target == &target;
		}

		inline bool AE_CALL operator==(const RefPtr<T>* ptr) const {
			return this == ptr;
		}

		inline bool AE_CALL operator!=(const T* target) const {
			return _target != target;
		}

		inline bool AE_CALL operator!=(const T& target) const {
			return _target != &target;
		}

		inline bool AE_CALL operator!=(const RefPtr<T>* ptr) const {
			return this != ptr;
		}

		inline AE_CALL operator bool() const {
			return _target;
		}

		template<typename S = T>
		inline S* AE_CALL get() const {
			return (S*)_target;
		}

		inline void AE_CALL set(T* target) {
			if (_target != target) {
				if (target) target->ref();
				if (_target) _target->unref();
				_target = target;
			}
		}

		inline void AE_CALL set(T& target) {
			if (_target != &target) {
				target.ref();
				if (_target) _target->unref();
				_target = &target;
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


	template<typename T>
	inline bool AE_CALL operator==(const T* lhs, const RefPtr<T>& rhs) {
		return rhs == lhs;
	}

	template<typename T>
	inline bool AE_CALL operator==(const T& lhs, const RefPtr<T>& rhs) {
		return rhs == &lhs;
	}

	template<typename T>
	inline bool AE_CALL operator!=(const T* lhs, const RefPtr<T>& rhs) {
		return rhs != lhs;
	}

	template<typename T>
	inline bool AE_CALL operator!=(const T& lhs, const RefPtr<T>& rhs) {
		return rhs != &lhs;
	}
}
