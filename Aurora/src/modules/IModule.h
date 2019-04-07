#pragma once

#include "base/Ref.h"
#include <any>
#include <string>
#include <unordered_map>

namespace aurora::modules {
	class AE_DLL ModuleType {
	public:
		ModuleType() = delete;
		ModuleType(const ModuleType&) = delete;
		ModuleType(ModuleType&&) = delete;

		static const ui32 GRAPHICS = 0b1;
		static const ui32 INPUT = 0b10;
	};


	class AE_TEMPLATE_DLL ModuleArgs {
	public:
		template<typename T>
		ModuleArgs& add(const std::string& name, const T value) {
			auto itr = _args.find(name);
			if (itr == _args.end()) {
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


	class AE_DLL IModule : public Ref {
	public:
		virtual ~IModule() {}

		virtual ui32 AE_CALL getType() const = 0;
	};
}