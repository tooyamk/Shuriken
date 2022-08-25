#pragma once

#include "Helper.h"

#include "Extensions/ASTCConverter/src/srk/ASTCConverter.h"
#include "Extensions/BC7Converter/src/srk/BC7Converter.h"
#include "Extensions/PNGConverter/src/srk/PNGConverter.h"
#include "Extensions/FBXConverter/src/srk/FBXConverter.h"
#include "Extensions/ShaderScript/src/srk/ShaderScript.h"

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