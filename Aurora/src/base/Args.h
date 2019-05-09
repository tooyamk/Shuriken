#pragma once

#include "base/LowLevel.h"
#include <any>
#include <unordered_map>

namespace aurora {
	class AE_TEMPLATE_DLL Args {
	public:
		template<typename T>
		Args& add(const std::string& name, const T value) {
			if (auto itr = _args.find(name); itr == _args.end()) {
				_args.emplace(name, value);
			} else {
				itr->second.emplace<T>(value);
			}

			return *this;
		}

		template<typename T>
		T get(const std::string& name, T defaultValue) const {
			auto itr = _args.find(name);
			return itr == _args.end() ? defaultValue : std::any_cast<T>(itr->second);
		}
	protected:
		std::unordered_map<std::string, std::any> _args;
	};
}