#pragma once

#include "srk/Application.h"
#include "srk/Literals.h"
#include "srk/Printer.h"
#include "srk/SerializableObject.h"
#include "srk/events/EventDispatcher.h"
#include "srk/lockfree/LinkedQueue.h"
#include "srk/lockfree/RingQueue.h"

#include "srk/Looper.h"
#include "srk/Time.h"
#include "srk/Material.h"
#include "srk/Monitor.h"
#include "srk/SceneNode.h"
#include "srk/ShaderPredefine.h"
#include "srk/ShaderParameter.h"
#include "srk/components/Camera.h"
#include "srk/components/lights/PointLight.h"
#include "srk/components/renderables/RenderableMesh.h"
#include "srk/modules/ModuleLoader.h"
#include "srk/modules/graphics/GraphicsAdapter.h"
#include "srk/render/ForwardRenderer.h"
#include "srk/render/StandardRenderPipeline.h"

#if __has_include("srk/ShaderTranspiler.h")
#	define SRK_HAS_SHADER_TRANSPILER_H
#	include "srk/ShaderTranspiler.h"
#endif
#include <fstream>

using namespace srk;
using namespace srk::components;
using namespace srk::components::lights;
using namespace srk::components::renderables;
using namespace srk::events;
using namespace srk::modules;
using namespace srk::modules::graphics;
using namespace srk::modules::inputs;
using namespace srk::modules::windows;
using namespace srk::render;

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
		size += 9;
	} else {
		size += 6;
	}

	std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<T>>, std::u8string, std::string> s;
	s.reserve(size);

	if constexpr (Environment::OPERATING_SYSTEM != Environment::OperatingSystem::WINDOWS) s += "lib";
	s += name;
	if constexpr (Environment::IS_DEBUG) s += 'd';
	s += '.';
	if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
		s += "dll";
	} else if (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::MACOS) {
		s += "dylib";
	} else {
		s += "so";
	}

	return s;
}

template<typename T>
requires ConvertibleString8Data<std::remove_cvref_t<T>>
inline std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<T>>, std::u8string, std::string> getDllPath(T&& name) {
	auto dll = getDllName(std::forward<T>(name));
#if SRK_OS == SRK_OS_ANDROID
	return dll;
#else
	return "libs/" + getDllName(std::forward<T>(name));
#endif
}

inline std::string getWindowDllPath() {
#if SRK_OS == SRK_OS_WINDOWS
	auto name = "win32api"sv;
#elif SRK_OS == SRK_OS_LINUX
	auto name = "x11"sv;
#elif SRK_OS == SRK_OS_MACOS
	auto name = "cocoa"sv;
#elif SRK_OS == SRK_OS_ANDROID
	auto name = "android-native"sv;
#else
	auto name = ""sv;
#endif
	return getDllPath("srk-module-window-" + name);
}

template<typename T>
requires ConvertibleString8Data<std::remove_cvref_t<T>>
inline ByteArray SRK_CALL readFile(T&& path) {
	ByteArray dst;
	std::ifstream stream(std::filesystem::path(path), std::ios::in | std::ios::ate | std::ios::binary);
	if (stream.is_open()) {
		auto size = stream.tellg();

		auto data = new uint8_t[size];

		stream.seekg(0, std::ios::beg);
		stream.read((char*)data, size);

		dst = ByteArray(data, size, ByteArray::Usage::EXCLUSIVE);
	}
	stream.close();
	return dst;
}

template<typename T>
requires ConvertibleString8Data<std::remove_cvref_t<T>>
inline bool SRK_CALL writeFile(T&& path, const ByteArray& data) {
	const char* p;
	if constexpr (String8Data<std::remove_cvref_t<T>>) {
		p = (const char*)path.data();
	} else {
		p = (const char*)path;
	}

	auto rst = false;
	ByteArray dst;
	std::ofstream stream(p, std::ios::out | std::ios::binary);
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
	return s;
}

#ifdef SRK_HAS_SHADER_TRANSPILER_H
inline IntrusivePtr<extensions::ShaderTranspiler> shaderTranspiler;
inline ProgramTranspileHandler programTranspileHandler = [](const ProgramTranspileInfo& info) {
	if (!shaderTranspiler) {
		shaderTranspiler = extensions::ShaderTranspiler::create(getDllPath("dxcompiler"));
		if (!shaderTranspiler) return ProgramSource();
	}

	extensions::ShaderTranspiler::Options opt;
	opt.spirv.descriptorSet0BindingOffset = 0;
	auto src = shaderTranspiler->translate(*info.source, opt, info.targetLanguage, info.targetVersion, info.defines, info.numDefines, info.includeHandler);
	if (src.isValid()) {
		printaln(L"------ translate shader code("sv, ProgramSource::toHLSLShaderStage(src.stage), L") ------\n"sv,
			std::string_view((char*)src.data.getSource(), src.data.getLength()), L"\n------------------------------------"sv);
	}
	return src;
};
#else
inline ProgramTranspileHandler programTranspileHandler = [](const ProgramTranspileInfo& info) {
	return ProgramSource();
};
#endif

inline bool SRK_CALL createProgram(IProgram& program, const std::string_view& vert, const std::string_view& frag) {
	using Str = std::remove_cvref_t<std::u8string>;
	using SSS = Str::value_type;
	using SSS2 = Str::traits_type;

	auto appPath = Application::getAppPath().parent_path().u8string() + "/Resources/shaders/";
	if (!program.create(readProgramSource(appPath + vert, ProgramStage::VS), readProgramSource(appPath + frag, ProgramStage::PS), nullptr, 0,
		[&appPath](const ProgramIncludeInfo& info) {
		return readFile(appPath + info.file);
	},
		[](const ProgramInputInfo& info) {
		return ProgramInputDescriptor();
	}, programTranspileHandler)) {
		printaln(L"program create error"sv);
		return false;
	}
	return true;
}

inline bool SRK_CALL isSupportTextureFormat(IGraphicsModule* g, TextureFormat fmt) {
	for (auto& i : g->getDeviceFeatures().textureFormats) {
		if (i == fmt) return true;
	}
	return false;
}

inline TextureFormat SRK_CALL textureFormatTypeSwitch(TextureFormat fmt, bool srgb) {
	switch (fmt) {
	case TextureFormat::R8G8B8_TYPELESS:
	case TextureFormat::R8G8B8_UNORM:
	case TextureFormat::R8G8B8_UNORM_SRGB:
	case TextureFormat::R8G8B8_UINT:
	case TextureFormat::R8G8B8_SNORM:
	case TextureFormat::R8G8B8_SINT:
		return srgb ? TextureFormat::R8G8B8_UNORM_SRGB : TextureFormat::R8G8B8_UNORM;
	case TextureFormat::R8G8B8A8_TYPELESS:
	case TextureFormat::R8G8B8A8_UNORM:
	case TextureFormat::R8G8B8A8_UNORM_SRGB:
	case TextureFormat::R8G8B8A8_UINT:
	case TextureFormat::R8G8B8A8_SNORM:
	case TextureFormat::R8G8B8A8_SINT:
		return srgb ? TextureFormat::R8G8B8A8_UNORM_SRGB : TextureFormat::R8G8B8A8_UNORM;
	case TextureFormat::BC1_TYPELESS:
	case TextureFormat::BC1_UNORM:
	case TextureFormat::BC1_UNORM_SRGB:
		return srgb ? TextureFormat::BC1_UNORM_SRGB : TextureFormat::BC1_UNORM;
	case TextureFormat::BC2_TYPELESS:
	case TextureFormat::BC2_UNORM:
	case TextureFormat::BC2_UNORM_SRGB:
		return srgb ? TextureFormat::BC2_UNORM_SRGB : TextureFormat::BC2_UNORM;
	case TextureFormat::BC3_TYPELESS:
	case TextureFormat::BC3_UNORM:
	case TextureFormat::BC3_UNORM_SRGB:
		return srgb ? TextureFormat::BC3_UNORM_SRGB : TextureFormat::BC3_UNORM;
	case TextureFormat::BC4_TYPELESS:
		return TextureFormat::BC4_UNORM;
	case TextureFormat::BC5_TYPELESS:
		return TextureFormat::BC5_UNORM;
	case TextureFormat::BC6_TYPELESS:
		return TextureFormat::BC6H_UF16;
	case TextureFormat::BC7_TYPELESS:
	case TextureFormat::BC7_UNORM:
	case TextureFormat::BC7_UNORM_SRGB:
		return srgb ? TextureFormat::BC7_UNORM_SRGB : TextureFormat::BC7_UNORM;
	default:
		return fmt;
	}
}