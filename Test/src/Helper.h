#pragma once

#include "srk/Framework.h"
#include "srk/Debug.h"
#include <fstream>

using namespace srk;
using namespace srk::components;
using namespace srk::events;
using namespace srk::modules;
using namespace srk::modules::graphics;
using namespace srk::modules::inputs;
using namespace srk::modules::windows;

using namespace std::literals;
using namespace srk::literals;
using namespace srk::enum_operators;

#ifdef __cpp_lib_generic_unordered_lookup
/*
struct std_generic_unordered_comparer {
	using is_transparent = void;
	template<typename K1, typename K2>
	inline bool SRK_CALL operator()(K1&& key1, K2&& key2) const {
		return key1 == key2;
	}
};

template<typename T>
struct transparent_hash {
	using is_transparent = void;
	template<typename K>
	inline size_t SRK_CALL operator()(K&& key) const {
		return std::hash<T>{}(key);
	}
};
*/
#endif

template<typename T>
requires ConvertibleString8Data<std::remove_cvref_t<T>>
inline std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<T>>, std::u8string, std::string> SRK_CALL getDllName(T&& name) {
	size_t size = 0;
	if constexpr (String8Data<T>) {
		size = name.size();
	} else {
		size = strlen((const char*)name);
	}
	if constexpr (Environment::IS_DEBUG) ++size;
	if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
		size += 4;
	} else if (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::MACOS) {
		size += 6;
	} else {
		size += 9;
	}

	std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<T>>, std::u8string, std::string> s;
	s.reserve(size);

	if constexpr (Environment::OPERATING_SYSTEM != Environment::OperatingSystem::WINDOWS) s += "lib";
	s += name;
	if constexpr (Environment::IS_DEBUG) s += '.';
	if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
		s += "dll";
	} else if (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::MACOS) {
		s += "dylib";
	} else {
		s += "so";
	}

	return std::move(s);
}

template<typename T>
requires ConvertibleString8Data<std::remove_cvref_t<T>>
inline std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<T>>, std::u8string, std::string> getDllPath(T&& name) {
	return "libs/" + getDllName(std::forward<T>(name));
}

inline std::string getWindowDllPath() {
#if SRK_OS == SRK_OS_WINDOWS
	auto name = "windows-classic"sv;
#elif SRK_OS == SRK_OS_LINUX
	auto name = "x11"sv;
#elif SRK_OS == SRK_OS_MACOS
	auto name = "cocoa"sv;
#else
	auto name = ""sv;
#endif
	return getDllPath("srk-windows-" + name);
}

template<typename T>
inline ByteArray SRK_CALL readFile(T&& path) {
	ByteArray dst;
	std::ifstream stream(std::filesystem::path(path), std::ios::in | std::ios::binary);
	if (stream.good()) {
		stream.seekg(0, std::ios::end);
		auto size = stream.tellg();

		auto data = new uint8_t[size];

		stream.seekg(0, std::ios::beg);
		stream.read((char*)data, size);

		dst = ByteArray(data, size, ByteArray::Usage::EXCLUSIVE);
	}
	stream.close();
	return std::move(dst);
}

inline bool SRK_CALL writeFile(const std::string& path, const ByteArray& data) {
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
inline ProgramSource SRK_CALL readProgramSource(T&& path, ProgramStage type) {
	ProgramSource s;
	s.language = ProgramLanguage::HLSL;
	s.stage = type;
	s.data = readFile(std::forward<T>(path));
	return std::move(s);
}

inline bool SRK_CALL createProgram(IProgram& program, const std::string_view& vert, const std::string_view& frag) {
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