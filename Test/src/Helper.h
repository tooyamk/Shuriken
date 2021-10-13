#pragma once

#include "aurora/Framework.h"
#include "aurora/Debug.h"
#include <fstream>

using namespace aurora;
using namespace aurora::components;
using namespace aurora::events;
using namespace aurora::modules;
using namespace aurora::modules::graphics;
using namespace aurora::modules::inputs;

using namespace std::literals;
using namespace aurora::literals;
using namespace aurora::enum_operators;

#ifdef __cpp_lib_generic_unordered_lookup
/*
struct std_generic_unordered_comparer {
	using is_transparent = void;
	template<typename K1, typename K2>
	inline bool AE_CALL operator()(K1&& key1, K2&& key2) const {
		return key1 == key2;
	}
};

template<typename T>
struct transparent_hash {
	using is_transparent = void;
	template<typename K>
	inline size_t AE_CALL operator()(K&& key) const {
		return std::hash<T>{}(key);
	}
};
*/
#endif

template<ConvertibleString8Data T>
inline auto AE_CALL getDLLName(T&& name) {
	size_t size = 0;
	if constexpr (String8Data<T>) {
		size = name.size();
	} else {
		size = strlen((const char*)name);
	}
	if constexpr (Environment::IS_DEBUG) ++size;
	if constexpr (Environment::OPERATING_SYSTEM != Environment::OperatingSystem::WINDOWS) {
		size += 4;
	} else {
		size += 6;
	}

	std::conditional_t<ConvertibleU8StringData<T>, std::u8string, std::string> s;
	s.reserve(size);

	if constexpr (Environment::IS_DEBUG) {
		if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
			s += name;
			s += "d.dll";
		} else {
			s += "lib";
			s += name;
			s += "d.so";
		}
	} else {
		if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
			s += name;
			s += ".dll";
		} else {
			s += "lib";
			s += name;
			s += ".so";
		}
	}

	return std::move(s);
}

template<typename T>
inline ByteArray AE_CALL readFile(T&& path) {
	ByteArray dst;
	std::ifstream stream(std::filesystem::path(path), std::ios::in | std::ios::binary);
	if (stream.good()) {
		auto beg = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto end = stream.tellg();
		auto size = end - beg;

		auto data = new uint8_t[size];

		stream.seekg(0, std::ios::beg);
		stream.read((char*)data, size);

		dst = ByteArray(data, size, ByteArray::Usage::EXCLUSIVE);
	}
	stream.close();
	return std::move(dst);
}

inline bool AE_CALL writeFile(const std::string& path, const ByteArray& data) {
	auto rst = false;
	ByteArray dst;
	std::ofstream stream(path, std::ios::out | std::ios::binary);
	if (stream.is_open()) {
		stream.write((const char*)data.getSource(), data.getLength());
		rst = true;
	}
	stream.close();
	return rst;
}

template<typename T>
inline ProgramSource AE_CALL readProgramSource(T&& path, ProgramStage type) {
	ProgramSource s;
	s.language = ProgramLanguage::HLSL;
	s.stage = type;
	s.data = readFile(std::forward<T>(path));
	return std::move(s);
}

inline bool AE_CALL createProgram(IProgram& program, const std::string_view& vert, const std::string_view& frag) {
	using Str = std::remove_cvref_t<std::u8string>;
	using SSS = Str::value_type;
	using SSS2 = Str::traits_type;

	auto appPath = getAppPath().parent_path().u8string() + "/Resources/shaders/";
	if (!program.create(readProgramSource(appPath + vert, ProgramStage::VS), readProgramSource(appPath + frag, ProgramStage::PS), nullptr, 0,
		[&appPath](const IProgram& program, ProgramStage stage, const std::string_view& name) {
		return readFile(appPath + name);
	},
		[](const IProgram& program, const std::string_view& name) {
		return modules::graphics::IProgram::InputDescription();
	})) {
		printaln(L"program create error");
		return false;
	}
	return true;
}