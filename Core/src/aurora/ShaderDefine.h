#pragma once

#include "aurora/Ref.h"
#include <unordered_map>

namespace aurora {
	class AE_DLL ShaderDefine {
	public:
		ShaderDefine() :
			name(nullptr),
			value(nullptr) {
		}

		std::string_view name;
		std::string_view value;
	};


	class AE_DLL IShaderDefineGetter : public Ref {
	public:
		virtual ~IShaderDefineGetter() {}

		virtual const std::string* AE_CALL get(const std::string& name) const = 0;
	};


	class AE_DLL ShaderDefineCollection : public IShaderDefineGetter {
	public:
		virtual ~ShaderDefineCollection() {}

		virtual const std::string* AE_CALL get(const std::string& name) const override;

		inline void AE_CALL add(const std::string& name, const std::string& value) {
			if (auto itr = _values.find(name); itr == _values.end()) {
				_values.emplace(name, value);
			} else {
				itr->second = value;
			}
		}
		inline void AE_CALL remove(const std::string& name) {
			if (auto itr = _values.find(name); itr != _values.end()) _values.erase(itr);
		}
		inline void AE_CALL clear() {
			_values.clear();
		}

	protected:
		std::unordered_map<std::string, std::string> _values;
	};


	class AE_DLL ShaderDefineGetterStack : public IShaderDefineGetter {
	public:
		virtual ~ShaderDefineGetterStack() {}

		virtual const std::string* AE_CALL get(const std::string& name) const override;

		inline bool AE_CALL push(IShaderDefineGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				return true;
			}
			return false;
		}
		inline bool AE_CALL push(IShaderDefineGetter& getter) {
			_stack.emplace_back(&getter);
			return true;
		}
		inline void AE_CALL pop() {
			_stack.pop_back();
		}
		inline void AE_CALL pop(size_t count) {
			_stack.erase(_stack.end() - count, _stack.end());
		}
		inline void AE_CALL clear() {
			_stack.clear();
		}

	protected:
		std::vector<RefPtr<IShaderDefineGetter>> _stack;
	};
}