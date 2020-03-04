#pragma once

#include "aurora/Aurora.h"
#include "Helper.h"

#include "Extensions/PNGConverter/src/aurora/PNGConverter.h"
#include "Extensions/FBXConverter/src/aurora/FBXConverter.h"
#include "Extensions/ShaderScript/src/aurora/ShaderScript.h"

using namespace aurora;
using namespace aurora::components;
using namespace aurora::components::renderables;
using namespace aurora::components::lights;
using namespace aurora::events;
using namespace aurora::modules;
using namespace aurora::modules::graphics;
using namespace aurora::modules::inputs;
using namespace aurora::render;

class Stats {
public:
	void AE_CALL run(Looper* looper);

private:
	std::atomic_uint32_t _frameCount = 0;
};


class BaseTester {
public:
	virtual int32_t AE_CALL run();
};