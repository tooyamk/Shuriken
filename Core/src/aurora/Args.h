#pragma once

#include "aurora/Global.h"
#include <any>
#include <optional>
#include <unordered_map>

namespace aurora {
	class AE_DLL Args {
	public:
		template<typename T>
		Args& add(const std::string& name, T&& val) {
			if constexpr (std::is_convertible_v<T, char const*> || std::is_same_v<T, std::string_view>) {
				add(name, std::string(val));
			} else if constexpr (std::is_convertible_v<T, wchar_t const*> || std::is_same_v<T, std::wstring_view>) {
				add(name, std::wstring(val));
			} else {
				_args.insert_or_assign(name, val);
			}

			return *this;
		}

		template<typename T>
		std::optional<T> get(const std::string& name) const {
			auto itr = _args.find(name);
			if (itr == _args.end()) {
				return std::nullopt;
			} else {
				try {
					return std::make_optional<T>(std::any_cast<T>(itr->second));
				} catch (const std::bad_any_cast&) {
					return std::nullopt;
				}
			}
		}
	protected:
		std::unordered_map<std::string, std::any> _args;
	};
}