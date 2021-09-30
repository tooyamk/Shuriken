#include "Shader.h"

namespace aurora {
	Shader::Shader() {
	}

	void Shader::set(modules::graphics::IGraphicsModule* graphics, ProgramSource* vs, ProgramSource* ps,
		const ShaderDefine* staticDefines, size_t numStaticDefines, const std::string_view* dynamicDefines, size_t numDynamicDefines,
		const IncludeHandler& includeHhandler, const InputHandler& inputHandler) {
		unset();

		_graphics = graphics;
		_vs = vs;
		_ps = ps;
		_includeHhandler = includeHhandler;
		_inputHandler = inputHandler;

		_defines.resize(numStaticDefines + numDynamicDefines);

		_staticDefines.resize(numStaticDefines << 1);
		for (decltype(numStaticDefines) i = 0; i < numStaticDefines; ++i) {
			auto sdef = staticDefines + i;
			auto& def = _defines[i];

			auto j = i << 1;
			_staticDefines[j] = sdef->name;
			def.name = _staticDefines[j];

			_staticDefines[++j] = sdef->value;
			def.value = _staticDefines[j];
		}

		_dynamicDefines.resize(numDynamicDefines);
		for (decltype(numDynamicDefines) i = 0; i < numDynamicDefines; ++i) _dynamicDefines[i] = *dynamicDefines;
	}

	void Shader::unset() {
		_graphics = nullptr;
		_vs = nullptr;
		_ps = nullptr;
		_programs.clear();
		_variants.clear();
	}

	IntrusivePtr<modules::graphics::IProgram> Shader::select(const IShaderDefineGetter* getter) {
		if (!_graphics) return nullptr;

		constexpr uint8_t NULL_VALUE = 0;
		constexpr uint16_t NO_VALUE = 0;

		hash::xxHash<64> hasher;
		hasher.begin(0);

		size_t count = _staticDefines.size() >> 1;
		if (getter) {
			for (uint16_t i = 0, n = _dynamicDefines.size(); i < n; ++i) {
				auto& d = _dynamicDefines[i];

				hasher.update(&i, sizeof(i));

				if (auto v = getter->get(d); v) {
					if (v->empty()) {
						hasher.update(&NULL_VALUE, sizeof(NULL_VALUE));
					} else {
						hasher.update(v->data(), v->size());
					}

					auto& def = _defines[count++];
					def.name = d;
					def.value = *v;
				} else {
					hasher.update(&NO_VALUE, sizeof(NO_VALUE));
				}
			}
		} else {
			for (uint16_t i = 0, n = _dynamicDefines.size(); i < n; ++i) {
				hasher.update(&i, sizeof(i));
				hasher.update(&NO_VALUE, sizeof(NO_VALUE));
			}
		}

		auto haslVal = hasher.digest();

		if (auto rst = _programs.emplace(haslVal, nullptr); rst.second) {
			if (auto p = _graphics->createProgram(); p) {
				if (auto itr2 = _variants.find(haslVal); itr2 == _variants.end()) {
					if (_vs && _ps) {
						if (auto succeeded = p->create(*_vs, *_ps, _defines.data(), count,
							[this](const modules::graphics::IProgram&, ProgramStage stage, const std::string_view& name) {
							return _includeHhandler ? _includeHhandler(*this, stage, name) : ByteArray();
						},
							[this](const modules::graphics::IProgram&, const std::string_view& name) {
							return _inputHandler ? _inputHandler(*this, name) : modules::graphics::IProgram::InputDescription();
						}); succeeded) {
							rst.first->second = p;

							return p;
						} else {
							return nullptr;
						}
					}
				} else {
					auto& variant = itr2->second;
					if (variant.vs && variant.ps) {
						if (auto succeeded = p->create(*variant.vs, *variant.ps, _defines.data(), count,
							[this](const modules::graphics::IProgram&, ProgramStage stage, const std::string_view& name) {
							return _includeHhandler ? _includeHhandler(*this, stage, name) : ByteArray();
						},
							[this](const modules::graphics::IProgram&, const std::string_view& name) {
							return _inputHandler ? _inputHandler(*this, name) : modules::graphics::IProgram::InputDescription();
						}); succeeded) {
							rst.first->second = p;

							return p;
						} else {
							return nullptr;
						}
					}
				}
			} else {
				return nullptr;
			}
		} else {
			return rst.first->second;
		}

		return nullptr;
	}

	void Shader::setVariant(ProgramSource* vs, ProgramSource* ps, const IShaderDefineGetter* getter) {
		if (_graphics) {
			constexpr uint8_t NULL_VALUE = 0;
			constexpr uint16_t NO_VALUE = 0;

			hash::xxHash<64> hasher;
			hasher.begin(0);

			for (uint16_t i = 0, n = _dynamicDefines.size(); i < n; ++i) {
				auto& d = _dynamicDefines[i];

				hasher.update(&i, sizeof(i));

				if (auto v = getter->get(d); v) {
					if (v->empty()) {
						hasher.update(&NULL_VALUE, sizeof(NULL_VALUE));
					} else {
						hasher.update(v->data(), v->size());
					}
				} else {
					hasher.update(&NO_VALUE, sizeof(NO_VALUE));
				}
			}

			auto& v = _variants.emplace(std::piecewise_construct, std::forward_as_tuple(hasher.digest()), std::forward_as_tuple()).first->second;
			v.vs = vs;
			v.ps = ps;
		}
	}
}