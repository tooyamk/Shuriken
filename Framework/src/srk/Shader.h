#pragma once

#include "srk/modules/graphics/GraphicsModule.h"
#ifndef __cpp_lib_generic_unordered_lookup
#	include <map>
#endif
#include <unordered_map>

namespace srk {
	class SRK_FW_DLL IShaderefineGetter : public Ref {
	public:
		virtual ~IShaderefineGetter() {}

		virtual const std::string* SRK_CALL get(const std::string_view& name) const = 0;
	};


	class SRK_FW_DLL ShaderDefineCollection : public IShaderefineGetter {
	public:
		ShaderDefineCollection() {}
		ShaderDefineCollection(const ShaderDefineCollection& other) : _values(other._values) {}
		virtual ~ShaderDefineCollection() {}

		inline void SRK_CALL operator=(const ShaderDefineCollection& other) {
			_values = other._values;
		}

		virtual const std::string* SRK_CALL get(const std::string_view& name) const override;

		inline void SRK_CALL set(const std::string_view& name, const std::string_view& value) {
			if (auto itr = _values.find(name); itr == _values.end()) {
				_values.emplace(name, value);
			} else {
				itr->second = value;
			}
		}
		inline bool SRK_CALL remove(const std::string_view& name) {
			if (auto itr = _values.find(name); itr != _values.end()) {
				_values.erase(itr);
				return true;
			}

			return false;
		}
		inline void SRK_CALL clear() {
			_values.clear();
		}

	protected:
#ifdef __cpp_lib_generic_unordered_lookup
		struct MapHasher {
			using is_transparent = void;
			template<typename K> inline size_t SRK_CALL operator()(K&& key) const { return std::hash<std::string_view>{}(key); }
		};
		std::unordered_map<std::string, std::string, MapHasher, std::equal_to<>>
#else
		std::map<std::string, std::string, std::less<>>
#endif
		_values;
	};


	class SRK_FW_DLL ShaderDefineGetterStack : public IShaderefineGetter {
	public:
		virtual ~ShaderDefineGetterStack() {}

		virtual const std::string* SRK_CALL get(const std::string_view& name) const override;

		inline bool SRK_CALL push(IShaderefineGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				return true;
			}
			return false;
		}
		template<std::derived_from<IShaderefineGetter>... Args>
		inline size_t SRK_CALL push(Args*... args) {
			size_t n = 0;
			((_push(n, args)), ...);
			return n;
		}
		inline bool SRK_CALL push(IShaderefineGetter& getter) {
			_stack.emplace_back(&getter);
			return true;
		}
		template<std::derived_from<IShaderefineGetter>... Args>
		inline size_t SRK_CALL push(Args&&... args) {
			((_stack.emplace_back(args)), ...);
			return sizeof...(args);
		}

		inline void SRK_CALL pop() {
			_stack.pop_back();
		}
		inline void SRK_CALL pop(size_t count) {
			_stack.erase(_stack.end() - count, _stack.end());
		}

		inline void SRK_CALL clear() {
			_stack.clear();
		}

	protected:
		std::vector<IntrusivePtr<IShaderefineGetter>> _stack;

		inline void SRK_CALL _push(size_t& n, IShaderefineGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				++n;
			}
		}
	};


	class SRK_FW_DLL Shader : public Ref {
	public:
		Shader();

		void SRK_CALL set(modules::graphics::IGraphicsModule* graphics, modules::graphics::ProgramSource* vs, modules::graphics::ProgramSource* ps,
			const modules::graphics::ProgramDefine* staticDefines, size_t numStaticDefines, const std::string_view* dynamicDefines, size_t numDynamicDefines,
			const modules::graphics::ProgramIncludeHandler& includeHandler, const modules::graphics::ProgramInputHandler& inputHandler, const modules::graphics::ProgramTranspileHandler& transpileHandler);
		IntrusivePtr<modules::graphics::IProgram> SRK_CALL select(const IShaderefineGetter* getter);

		void SRK_CALL unset();

		void SRK_CALL setVariant(modules::graphics::ProgramSource* vs, modules::graphics::ProgramSource* ps, const IShaderefineGetter* getter);

	protected:
		struct Variant {
			IntrusivePtr<modules::graphics::ProgramSource> vs;
			IntrusivePtr<modules::graphics::ProgramSource> ps;
		};


		IntrusivePtr<modules::graphics::IGraphicsModule> _graphics;
		IntrusivePtr<modules::graphics::ProgramSource> _vs;
		IntrusivePtr<modules::graphics::ProgramSource> _ps;
		modules::graphics::ProgramIncludeHandler _includeHhandler;
		modules::graphics::ProgramInputHandler _inputHandler;
		modules::graphics::ProgramTranspileHandler _transpileHandler;

		std::unordered_map<uint64_t, Variant> _variants;

		std::vector<std::string> _staticDefines;
		std::vector<std::string> _dynamicDefines;
		std::vector<modules::graphics::ProgramDefine> _defines;

		std::unordered_map<uint64_t, IntrusivePtr<modules::graphics::IProgram>> _programs;
	};
}