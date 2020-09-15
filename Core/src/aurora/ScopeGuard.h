#pragma once

#include <functional>

namespace aurora {
	class AE_CORE_DLL ScopeGuard {
	public:
		template<typename Fn>
		ScopeGuard(Fn&& fn) : _fn(std::forward<Fn>(fn)) {}

		ScopeGuard(ScopeGuard&& other) : _fn(std::move(other._fn)) {
			other._fn = nullptr;
		}

		~ScopeGuard() {
			if (_fn) _fn();
		}

		ScopeGuard(const ScopeGuard&) = delete;
		void operator=(const ScopeGuard&) = delete;
	private:
		std::function<void()> _fn;
	};
}