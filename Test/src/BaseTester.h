#pragma once

#include "Helper.h"

#if __has_include("srk/AndroidNativeApplication.h")
#	define SRK_HAS_ANDROID_NATIVE_APPLICATION_H
#	include "srk/AndroidNativeApplication.h"
#endif
#if __has_include("srk/ASTCConverter.h")
#	define SRK_HAS_ASTC_CONVERTER_H
#	include "srk/ASTCConverter.h"
#endif
#if __has_include("srk/BC7Converter.h")
#	define SRK_HAS_BC7_CONVERTER_H
#	include "srk/BC7Converter.h"
#endif
#if __has_include("srk/FBXConverter.h")
#	define SRK_HAS_FBX_CONVERTER_H
#	include "srk/FBXConverter.h"
#endif
#if __has_include("srk/PNGConverter.h")
#	define SRK_HAS_PNG_CONVERTER_H
#	include "srk/PNGConverter.h"
#endif
#if __has_include("srk/JPEGConverter.h")
#	define SRK_HAS_JPEG_CONVERTER_H
#	include "srk/JPEGConverter.h"
#endif
#if __has_include("srk/ShaderScript.h")
#	define SRK_HAS_SHADER_SCRIPT_H
#	include "srk/ShaderScript.h"
#endif

using namespace srk;
using namespace srk::components;
using namespace srk::components::renderables;
using namespace srk::components::lights;
using namespace srk::events;
using namespace srk::modules;
using namespace srk::modules::graphics;
using namespace srk::modules::inputs;
using namespace srk::render;

class Stats {
public:
	void SRK_CALL run(Looper* looper);

private:
	std::atomic_uint32_t _frameCount = 0;
};


class BaseTester {
public:
	virtual int32_t SRK_CALL run();
};