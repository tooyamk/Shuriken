#pragma once

#include "aurora/modules/inputs/IInputModule.h"
#include "aurora/IApplication.h"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")