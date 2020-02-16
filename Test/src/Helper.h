#pragma once

#include "aurora/Aurora.h"
#include <fstream>

using namespace aurora;
using namespace aurora::components;
using namespace aurora::events;
using namespace aurora::modules;
using namespace aurora::modules::graphics;
using namespace aurora::modules::inputs;

inline std::string getDLLName(const std::string& name) {
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

inline ByteArray readFile(const std::string& path) {
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

inline ProgramSource readProgramSource(const std::string& path, ProgramStage type) {
	ProgramSource s;
	s.language = ProgramLanguage::HLSL;
	s.stage = type;
	s.data = readFile(path);
	return std::move(s);
}

inline bool programCreate(IProgram& program, const std::string_view& vert, const std::string_view& frag) {
	std::string appPath = String::UnicodeToUtf8(getAppPath()) + u8"Resources/shaders/";
	if (program.create(readProgramSource(appPath + vert.data(), ProgramStage::VS), readProgramSource(appPath + frag.data(), ProgramStage::PS), nullptr, 0,
		[&appPath](const IProgram& program, ProgramStage stage, const std::string_view& name) {
		return readFile(appPath + name.data());
	})) {
		println(L"program create error");
		return false;
	}
	return true;
}