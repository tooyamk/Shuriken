#pragma once

#include "aurora/Global.h"
#include <atomic>

namespace aurora {
	template<typename T>
	class RefPtr {
	public:
		RefPtr() :
			_target(nullptr) {
		}

		RefPtr(const RefPtr<T>& ptr) : RefPtr(ptr._target) {
		}

		RefPtr(RefPtr<T>&& ptr) noexcept :
			_target(ptr._target) {
			ptr._target = nullptr;
		}

		RefPtr(T* target) :
			_target(target ? target->template ref<T>() : nullptr) {
		}

		RefPtr(T& target) :
			_target(target.template ref<T>()) {
		}

		~RefPtr() {
			reset();
		}

		inline T* AE_CALL operator->() const {
			return _target;
		}

		inline T& AE_CALL operator*() const {
			return *_target;
		}

		inline RefPtr<T>& AE_CALL operator=(RefPtr<T>&& ptr) noexcept {
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

		inline bool AE_CALL operator==(const T& target) const {
			return _target == &target;
		}

		inline bool AE_CALL operator==(const RefPtr<T>* ptr) const {
			return this == ptr;
		}

		inline bool AE_CALL operator!=(const T& target) const {
			return _target != &target;
		}

		inline bool AE_CALL operator!=(const RefPtr<T>* ptr) const {
			return this != ptr;
		}

		inline AE_CALL operator T* () const {
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

		template<bool DoUnref = true>
		inline void AE_CALL reset() {
			if (_target) {
				if constexpr (DoUnref) {
					_target->unref();
				}
				_target = nullptr;
			}
		}

		inline bool AE_CALL isEmpty() const {
			return !_target;
		}

	private:
		T* _target;
	};


	template<typename T>
	inline bool AE_CALL operator==(const T& lhs, const RefPtr<T>& rhs) {
		return rhs == &lhs;
	}

	template<typename T>
	inline bool AE_CALL operator!=(const T& lhs, const RefPtr<T>& rhs) {
		return rhs != &lhs;
	}


	class AE_CORE_DLL Ref {
	public:
		Ref() :
			_refCount(0) {
		}

		virtual ~Ref() {
		}

		inline void AE_CALL ref() {
			_refCount.fetch_add(1, std::memory_order_release);
		}

		template<typename T>
		inline T* AE_CALL ref() {
			ref();
			return (T*)this;
		}

		template<bool CheckRelease = true>
		inline RefPtr<Ref> AE_CALL unref() {
			if constexpr (CheckRelease) {
				if (_refCount.fetch_sub(1) <= 1) {
					auto rst = _destruct();
					delete this;
					return std::move(rst);
				}
			} else {
				_refCount.fetch_sub(1);
			}

			return nullptr;
		}

		inline uint32_t AE_CALL getReferenceCount() const {
			return _refCount.load(std::memory_order_acquire);
		}

		template<typename P1, typename P2>
		inline static void AE_CALL set(P1*& ptr, P2* target) {
			if (ptr != target) {
				if (target) target->ref();
				if (ptr) ptr->unref();
				ptr = target;
			}
		}

		template<typename P>
		inline static void AE_CALL setNull(P*& ptr) {
			ptr->unref();
			ptr = nullptr;
		}

		template<typename P>
		inline static void AE_CALL checkSetNull(P*& ptr) {
			if (ptr) setNull(ptr);
		}

	protected:
		std::atomic_uint32_t _refCount;

		virtual RefPtr<Ref> AE_CALL _destruct() { return nullptr; }
	};
}
