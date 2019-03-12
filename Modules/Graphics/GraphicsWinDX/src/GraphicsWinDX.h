#pragma once

#include "modules/GraphicsModule.h"

AE_NS_BEGIN

class AE_MODULE_DLL GraphicsWinDX : public GraphicsModule {
public:
	virtual void AE_CALL createView(void* style, const i8* windowTitle, i32 x, i32 y, i32 w, i32 h);
};

AE_NS_END

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new AE_NS::GraphicsWinDX();
}
#endif