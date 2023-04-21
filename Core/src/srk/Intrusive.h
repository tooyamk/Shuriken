#pragma once

#include "srk/Core.h"
#include <atomic>

#define SRK_REF_OBJECT(__CLASS__) \
protected: \
	std::atomic_uint32_t _refCount = 0; \
public: \
	inline uint32_t SRK_CALL getReferenceCount() const { return _refCount.load(std::memory_order::relaxed); } \
	inline void SRK_CALL ref() { _refCount.fetch_add(1, std::memory_order::relaxed); } \
	template<typename T> \
	inline T* SRK_CALL ref() { \
		ref(); \
		return (T*)this; \
	} \
	template<bool AutoDelete = true> \
	inline static void SRK_CALL unref(__CLASS__& target) { \
		if constexpr (AutoDelete) { \
			if (target._refCount.fetch_sub(1, std::memory_order::acquire) <= 1) { delete &target; } \
		} else { \
			target._refCount.fetch_sub(1, std::memory_order::relaxed); \
		} \
	} \
private:

namespace srk {
	template<typename T>
	concept IntrusivePtrOperableObject = requires(T& t) {
		t.ref();
		std::remove_cvref_t<T>::unref(t);
	};


	class SRK_CORE_DLL Ref {
		SRK_REF_OBJECT(Ref)
	public:
		virtual ~Ref() {
		}
	};


	template<IntrusivePtrOperableObject T>
	inline void SRK_CALL intrusivePtrAddRef(T& val) {
		val.ref();
	}

	template<IntrusivePtrOperableObject T>
	inline void SRK_CALL intrusivePtrRelease(T& val) {
		std::remove_cvref_t<T>::unref(val);
	}


	template<typename T>
	class IntrusivePtr {
	public:
		using ValueType = T;

		IntrusivePtr() noexcept :
			_target(nullptr) {
		}

		IntrusivePtr(std::nullptr_t) noexcept :
			_target(nullptr) {
		}

		IntrusivePtr(const IntrusivePtr<T>& ptr) : IntrusivePtr(ptr._target) {
		}

		IntrusivePtr(IntrusivePtr<T>&& ptr) noexcept :
			_target(ptr._target) {
			ptr._target = nullptr;
		}

		IntrusivePtr(T* target) :
			_target(target) {
			if (_target) intrusivePtrAddRef(*_target);
		}

		IntrusivePtr(T& target) :
			_target(&target) {
			intrusivePtrAddRef(target);
		}

		~IntrusivePtr() {
			reset();
		}

		inline T* SRK_CALL operator->() const {
			return _target;
		}

		inline T& SRK_CALL operator*() const {
			return *_target;
		}

		inline IntrusivePtr<T>& SRK_CALL operator=(IntrusivePtr<T>&& ptr) noexcept {
			if (_target) intrusivePtrRelease(*_target);
			_target = ptr._target;
			ptr._target = nullptr;
			return *this;
		}

		inline void SRK_CALL operator=(T* target) {
			set(target);
		}

		inline void SRK_CALL operator=(T& target) {
			set(target);
		}

		inline void SRK_CALL operator=(const IntrusivePtr<T>& ptr) {
			set(ptr._target);
		}

		inline void SRK_CALL operator=(std::nullptr_t) {
			reset();
		}

		inline bool SRK_CALL operator==(const T& target) const {
			return _target == &target;
		}

		inline bool SRK_CALL operator==(const IntrusivePtr<T>* ptr) const {
			return this == ptr;
		}

		inline bool SRK_CALL operator==(std::nullptr_t) const {
			return !_target;
		}

		inline bool SRK_CALL operator!=(const T& target) const {
			return _target != &target;
		}

		inline bool SRK_CALL operator!=(const IntrusivePtr<T>* ptr) const {
			return this != ptr;
		}

		inline bool SRK_CALL operator!=(std::nullptr_t) const {
			return _target;
		}

		inline SRK_CALL operator T*() const {
			return _target;
		}

		inline uintptr_t uintptr() const {
			return (uintptr_t)_target;
		}

		template<typename S = T>
		inline S* SRK_CALL get() const {
			return (S*)_target;
		}

		inline void SRK_CALL set(std::nullptr_t) {
			reset();
		}

		inline void SRK_CALL set(T* target) {
			if (_target != target) {
				if (target) intrusivePtrAddRef(*target);
				if (_target) intrusivePtrRelease(*_target);
				_target = target;
			}
		}

		inline void SRK_CALL set(T& target) {
			set(&target);
		}

		template<bool DoUnref = true>
		inline void SRK_CALL reset() {
			if (_target) {
				if constexpr (DoUnref) {
					intrusivePtrRelease(*_target);
				}
				_target = nullptr;
			}
		}

		inline bool SRK_CALL isEmpty() const {
			return !_target;
		}

	private:
		T* _target;
	};


	template<typename T>
	inline bool SRK_CALL operator==(const T& lhs, const IntrusivePtr<T>& rhs) {
		return rhs == &lhs;
	}

	template<typename T>
	inline bool SRK_CALL operator!=(const T& lhs, const IntrusivePtr<T>& rhs) {
		return rhs != &lhs;
	}
}
