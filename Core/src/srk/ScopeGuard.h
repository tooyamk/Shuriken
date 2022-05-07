#pragma once

namespace srk {
	template <typename Fn>
	class ScopeGuard {
	public:
		ScopeGuard() = delete;
		ScopeGuard(const ScopeGuard&) = delete;
		ScopeGuard& operator=(const ScopeGuard&) = delete;

		ScopeGuard(Fn&& f) : 
			_fun(std::forward<Fn>(f)),
			_active(true) {
		}

		ScopeGuard(ScopeGuard&& other) : 
			_fun(std::move(other._fun)),
			_active(other._active) {
			other.dismiss();
		}

		~ScopeGuard() {
			if (_active) _fun();
		}

		inline void SRK_CALL dismiss() {
			_active = false;
		}

	private:
		Fn _fun;
		bool _active;
	};
}