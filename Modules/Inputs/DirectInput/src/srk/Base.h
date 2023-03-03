#pragma once

#include "srk/modules/inputs/InputModule.h"
#include <shared_mutex>

#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE(p) { if (p) { (p)->Release();  (p) = nullptr; } }
#endif