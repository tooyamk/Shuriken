#pragma once

#include "aurora/modules/inputs/IInputModule.h"
#include "aurora/IApplication.h"
#include <shared_mutex>

#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE(p) { if (p) { (p)->Release();  (p) = nullptr; } }
#endif