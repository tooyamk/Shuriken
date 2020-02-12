#pragma once

#include "aurora/Global.h"
#include <any>
#include <optional>
#include <unordered_map>

namespace aurora {
	class AE_TEMPLATE_DLL Args {
	public:
		template<typename T>
		Args& add(const std::string& name, const T& val) {
			if constexpr (std::is_convertible_v<T, char const*> || std::is_same_v<T, std::string_view>) {
				add(name, std::string(val));
			} else {
				auto itr = _args.find(name);
				if (itr == _args.end()) {
					_args.emplace(name, val);
				} else {
					itr->second = val;
				}
			}

			return *this;
		}

		template<typename T>
		std::optional<T> get(const std::string& name) const {
			auto itr = _args.find(name);
			if (itr == _args.end()) {
				return std::nullopt;
			} else {
				return itr->second.type() == typeid(T) ? std::make_optional<T>(std::any_cast<T>(itr->second)) : std::nullopt;
			}
		}
	protected:
		std::unordered_map<std::string, std::any> _args;
	};
}