#pragma once

#include "Aurora.h"
#include <fstream>

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

inline aurora::ByteArray readFile(const std::string& path) {
	aurora::ByteArray dst;
	std::ifstream stream(path, std::ios::in | std::ios::binary);
	if (stream.good()) {
		auto beg = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto end = stream.tellg();
		auto size = end - beg;

		auto data = new uint8_t[size];

		stream.seekg(0, std::ios::beg);
		stream.read((char*)data, size);

		dst = aurora::ByteArray(data, size, aurora::ByteArray::Usage::EXCLUSIVE);
	}
	stream.close();
	return std::move(dst);
}

inline aurora::modules::graphics::ProgramSource readProgramSourcee(const std::string& path, aurora::modules::graphics::ProgramStage type) {
	aurora::modules::graphics::ProgramSource s;
	s.language = aurora::modules::graphics::ProgramLanguage::HLSL;
	s.stage = type;
	s.data = readFile(path);
	return std::move(s);
}