#pragma once

#include "modules/GraphicsModule.h"

AE_NS_BEGIN
#ifdef AE_EXPORTS
#define AE_DLL AE_DLL_EXPORT 
#else
#define AE_DLL AE_DLL_IMPORT 
#endif
class GraphicsWinDX : public GraphicsModule {
public:
	virtual void AE_CALL aaa();
};

AE_NS_END

extern "C" __declspec(dllexport) void* createModule() {
	return new AE_NS::GraphicsWinDX();
}