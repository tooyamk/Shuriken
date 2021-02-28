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

template<typename... Args>
inline void AE_CALL printaln(Args&&... args) {
#if AE_OS == AE_OS_WIN
	if (IsDebuggerPresent()) {
		Debug::print<Debug::DebuggerOutput>(std::forward<Args>(args)..., L"\n");
	} else {
		Debug::print<Debug::ConsoleOutput>(std::forward<Args>(args)..., L"\n");
	}
#else
	Debug::print<Debug::ConsoleOutput>(std::forward<Args>(args)..., L"\n");
#endif
}

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

inline std::string AE_CALL getDLLName(const std::string& name) {
	if constexpr (environment::is_debug) {
		if constexpr (environment::current_operating_system == environment::operating_system::windows) {
			return name + "d.dll";
		} else {
			return "lib" + name + "d.so";
		}
	} else {
		if constexpr (environment::current_operating_system == environment::operating_system::windows) {
			return name + ".dll";
		} else {
			return "lib" + name + ".so";
		}
	}
}

template<typename T>
inline ByteArray AE_CALL readFile(T&& path) {
	ByteArray dst;
	std::ifstream stream(path, std::ios::in | std::ios::binary);
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
	auto appPath = getAppPath().parent_path().u8string() + "/Resources/shaders/";
	if (!program.create(readProgramSource(appPath + vert, ProgramStage::VS), readProgramSource(appPath + frag, ProgramStage::PS), nullptr, 0,
		[&appPath](const IProgram& program, ProgramStage stage, const std::string_view& name) {
		return readFile(appPath + name);
	})) {
		printaln(L"program create error");
		return false;
	}
	return true;
}