#pragma once

#include "aurora/Intrusive.h"
#include <unordered_map>
#include <vector>

namespace aurora {
	class AE_FW_DLL ShaderDefine {
	public:
		ShaderDefine() {
		}

		ShaderDefine(const std::string_view& name, const std::string_view& value) :
			name(name),
			value(value) {
		}

		std::string_view name;
		std::string_view value;
	};


	class AE_FW_DLL IShaderDefineGetter : public Ref {
	public:
		virtual ~IShaderDefineGetter() {}

		virtual const std::string* AE_CALL get(const QueryString& name) const = 0;
	};


	class AE_FW_DLL ShaderDefineCollection : public IShaderDefineGetter {
	public:
		ShaderDefineCollection() {}
		ShaderDefineCollection(const ShaderDefineCollection& other) : _values(other._values) {}
		virtual ~ShaderDefineCollection() {}

		inline void AE_CALL operator=(const ShaderDefineCollection& other) {
			_values = other._values;
		}

		virtual const std::string* AE_CALL get(const QueryString& name) const override;

		inline void AE_CALL set(const QueryString& name, const std::string_view& value) {
			if (auto itr = _values.find(name); itr == _values.end()) {
				_values.emplace(name, value);
			} else {
				itr->second = value;
			}
		}
		inline bool AE_CALL remove(const QueryString& name) {
			if (auto itr = _values.find(name); itr != _values.end()) {
				_values.erase(itr);
				return true;
			}

			return false;
		}
		inline void AE_CALL clear() {
			_values.clear();
		}

	protected:
		StringUnorderedMap<std::string> _values;
	};


	class AE_FW_DLL ShaderDefineGetterStack : public IShaderDefineGetter {
	public:
		virtual ~ShaderDefineGetterStack() {}

		virtual const std::string* AE_CALL get(const QueryString& name) const override;

		inline bool AE_CALL push(IShaderDefineGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				return true;
			}
			return false;
		}
		template<std::derived_from<IShaderDefineGetter>... Args>
		inline size_t AE_CALL push(Args*... args) {
			size_t n = 0;
			((_push(n, args)), ...);
			return n;
		}
		inline bool AE_CALL push(IShaderDefineGetter& getter) {
			_stack.emplace_back(&getter);
			return true;
		}
		template<std::derived_from<IShaderDefineGetter>... Args>
		inline size_t AE_CALL push(Args&&... args) {
			((_stack.emplace_back(args)), ...);
			return sizeof...(args);
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
		std::vector<IntrusivePtr<IShaderDefineGetter>> _stack;

		inline void AE_CALL _push(size_t& n, IShaderDefineGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				++n;
			}
		}
	};
}