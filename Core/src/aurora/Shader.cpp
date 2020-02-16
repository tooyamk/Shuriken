#include "Shader.h"

namespace aurora {
	Shader::Shader() :
		_isResourcesValid(false) {
	}

	void Shader::upload(modules::graphics::IGraphicsModule* graphics, ProgramSource* vs, ProgramSource* ps,
		const ShaderDefine* staticDefines, size_t numStaticDefines, const std::string_view* dynamicDefines, size_t numDynamicDefines,
		const IncludeHandler& handler) {
		_graphics = graphics;
		_vs = vs;
		_ps = ps;
		_includeHhandler = handler;

		_isResourcesValid = _graphics && _vs && _ps;

		_defines.resize(numStaticDefines + numDynamicDefines);

		_staticDefines.resize(numStaticDefines << 1);
		for (size_t i = 0; i < numStaticDefines; ++i) {
			auto sdef = staticDefines + i;
			auto& def = _defines[i];

			auto j = i << 1;
			_staticDefines[j] = sdef->name;
			def.name = _staticDefines[j];

			++j;
			_staticDefines[j] = sdef->value;
			def.value = _staticDefines[j];
		}

		_dynamicDefines.resize(numDynamicDefines);
		for (size_t i = 0; i < numDynamicDefines; ++i) _dynamicDefines[i] = *dynamicDefines;
	}

	void Shader::unload() {
		_graphics = nullptr;
		_vs = nullptr;
		_ps = nullptr;
		_programs.clear();
		_isResourcesValid = false;
	}

	modules::graphics::IProgram* Shader::select(const IShaderDefineGetter* getter) {
		if (!_isResourcesValid) return nullptr;

		constexpr uint8_t NULL_VALUE = 0;
		constexpr uint16_t NO_VALUE = 0;

		uint64_t fv = 0;

		size_t count = _staticDefines.size() >> 1;
		if (getter) {
			for (size_t i = 0, n = _dynamicDefines.size(); i < n; ++i) {
				auto& d = _dynamicDefines[i];

				fv = hash::CRC::update<64, false>(fv, &i, sizeof(i), _crcTable);

				if (auto v = getter->get(d); v) {
					if (auto size = v->size(); size) {
						fv = hash::CRC::update<64, false>(fv, v->data(), v->size(), _crcTable);
					} else {
						fv = hash::CRC::update<64, false>(fv, &NULL_VALUE, sizeof(NULL_VALUE), _crcTable);
					}

					auto& def = _defines[count++];
					def.name = d;
					def.value = *v;
				} else {
					fv = hash::CRC::update<64, false>(fv, &NO_VALUE, sizeof(NO_VALUE), _crcTable);
				}
			}
		} else {
			for (size_t i = 0, n = _dynamicDefines.size(); i < n; ++i) {
				fv = hash::CRC::update<64, false>(fv, &i, sizeof(i), _crcTable);
				fv = hash::CRC::update<64, false>(fv, &NO_VALUE, sizeof(NO_VALUE), _crcTable);
			}
		}

		fv = hash::CRC::finish<64, false>(fv, 0);

		if (auto itr = _programs.find(fv); itr == _programs.end()) {
			if (RefPtr p = _graphics->createProgram(); p) {
				auto succeeded = p->create(*_vs, *_ps, _defines.data(), count, [this](const modules::graphics::IProgram&, ProgramStage stage, const std::string_view& name) {
					return _includeHhandler ? _includeHhandler(*this, stage, name) : ByteArray();
				});
				if (succeeded) _programs.emplace(fv, p);
			} else {
				return nullptr;
			}
		} else {
			return itr->second;
		}

		return nullptr;
	}
}