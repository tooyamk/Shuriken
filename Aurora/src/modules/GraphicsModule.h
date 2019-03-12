#pragma once

#include "modules/Module.h"

AE_NS_BEGIN

class AE_DLL GraphicsModule {
public:
	virtual void AE_CALL createView(void* style, const i8* windowTitle, i32 x, i32 y, i32 w, i32 h) = 0;
};

AE_NS_END