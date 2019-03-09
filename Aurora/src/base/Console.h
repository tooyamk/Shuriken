#pragma once

#include "Aurora.h"

#ifdef WIN32
#include "atlbase.h"
#include "atlstr.h"
#endif

AE_NS_BEGIN

void print(const i8* msg, ...) {

#ifdef WIN32
	i8 strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, msg);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, msg, vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
#endif

}

AE_NS_END