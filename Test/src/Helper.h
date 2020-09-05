#pragma once

#include "aurora/Framework.h"
#include <fstream>

using namespace aurora;
using namespace aurora::components;
using namespace aurora::events;
using namespace aurora::modules;
using namespace aurora::modules::graphics;
using namespace aurora::modules::inputs;

struct std_string_unordered_comparer {
	using is_transparent = void;
	inline bool AE_CALL operator()(const std::string& key1, const std::string_view& key2) const {
		return key1 == key2;
	}
	inline bool AE_CALL operator()(const std::string_view& key1, const std::string_view& key2) const {
		return key1 == key2;
	}
};

struct std_string_unordered_hasher {
	using is_transparent = void;
	using transparent_key_equal = std_string_unordered_comparer;
	inline size_t AE_CALL operator()(const std::string_view& key) const {
		return std::hash<std::string_view>{}(key);
	}
	inline size_t AE_CALL operator()(const std::string& key) const {
		return std::hash<std::string>{}(key);
	}
};

inline std::string AE_CALL getDLLName(const std::string& name) {
#if defined(AE_DEBUG)
#if AE_OS == AE_OS_WIN
	return name + "d.dll";
#else
	return name + "d.so";
#endif
#else
#if AE_OS == AE_OS_WIN
	return name + ".dll";
#else
	return name + ".so";
#endif
#endif
}

inline ByteArray AE_CALL readFile(const std::string& path) {
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

inline ProgramSource AE_CALL readProgramSource(const std::string& path, ProgramStage type) {
	ProgramSource s;
	s.language = ProgramLanguage::HLSL;
	s.stage = type;
	s.data = readFile(path);
	return std::move(s);
}

inline bool AE_CALL programCreate(IProgram& program, const std::string_view& vert, const std::string_view& frag) {
	std::string appPath = getAppPath().parent_path().u8string() + "/Resources/shaders/";
	if (program.create(readProgramSource(appPath + vert.data(), ProgramStage::VS), readProgramSource(appPath + frag.data(), ProgramStage::PS), nullptr, 0,
		[&appPath](const IProgram& program, ProgramStage stage, const std::string_view& name) {
		return readFile(appPath + name.data());
	})) {
		println(L"program create error");
		return false;
	}
	return true;
}