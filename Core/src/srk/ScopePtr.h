#pragma once

#include "srk/Global.h"

namespace srk {
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

		inline T* SRK_CALL operator->() const {
			return _target;
		}

		inline T& SRK_CALL operator*() const {
			return *_target;
		}

		inline void SRK_CALL operator=(T* target) {
			set(target);
		}

		inline void SRK_CALL operator=(std::nullptr_t) {
			reset();
		}

		inline SRK_CALL operator T* () const {
			return _target;
		}

		inline void SRK_CALL set(std::nullptr_t) {
			reset();
		}

		inline void SRK_CALL set(T* target) {
			if (_target != target) {
				if (_target) delete _target;
				_target = target;
			}
		}

		template<bool DoRelease = true>
		inline void SRK_CALL reset() {
			if (_target) {
				if constexpr (DoRelease) {
					delete _target;
				}
				_target = nullptr;
			}
		}

		inline T* SRK_CALL detach() {
			auto p = _target;
			_target = nullptr;
			return p;
		}

		inline bool SRK_CALL isEmpty() const {
			return !_target;
		}

	private:
		T* _target;
	};
}