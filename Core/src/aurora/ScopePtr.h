#pragma once

#include "aurora/Global.h"

namespace aurora {
	template<typename T>
	class ScopePtr {
	public:
		ScopePtr(const ScopePtr& ptr) = delete;
		ScopePtr& operator=(const ScopePtr&) = delete;

		ScopePtr() noexcept :
			_target(nullptr) {
		}

		ScopePtr(std::nullptr_t) noexcept :
			_target(nullptr) {
		}

		ScopePtr(ScopePtr&& ptr) noexcept :
			_target(ptr._target) {
			ptr._target = nullptr;
		}

		ScopePtr(T* target) :
			_target(target) {
		}

		~ScopePtr() {
			reset();
		}

		inline T* AE_CALL operator->() const {
			return _target;
		}

		inline T& AE_CALL operator*() const {
			return *_target;
		}

		inline AE_CALL operator T* () const {
			return _target;
		}

		inline void AE_CALL set(std::nullptr_t) {
			reset();
		}

		inline void AE_CALL set(T* target) {
			if (_target != target) {
				if (_target) delete _target;
				_target = target;
			}
		}

		template<bool DoRelease = true>
		inline void AE_CALL reset() {
			if (_target) {
				if constexpr (DoRelease) {
					delete _target;
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
}