#pragma once

#include "aurora/Aurora.h"
#include "Helper.h"

#include "../../Extensions/Files/Images/PNGConverter/src/aurora/PNGConverter.h"

using namespace aurora;
using namespace aurora::components;
using namespace aurora::events;
using namespace aurora::modules;
using namespace aurora::modules::graphics;
using namespace aurora::modules::inputs;

class Stats {
public:
	void AE_CALL run(Application* app);

private:
	std::atomic_uint32_t _frameCount = 0;
};


class BaseTester {
public:
	virtual int32_t AE_CALL run();
};