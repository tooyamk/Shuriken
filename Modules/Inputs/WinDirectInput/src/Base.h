#pragma once

#include "modules/IInputModule.h"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "Aurora.lib") 