#include "Entry.h"

#if SRK_OS != SRK_OS_MACOS
#	if SRK_OS == SRK_OS_WINDOWS
//#pragma comment(linker, "/subsystem:console")
int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int32_t nCmdShow) {
	SetDllDirectoryW((Application::getAppPath().parent_path().wstring() + L"/libs/").data());

	Enttry e;
	return e.run();
}
#	elif SRK_OS == SRK_OS_ANDROID
#include "AndroidApp.h"

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    new AndroidApp(activity);
}
#	else
int32_t main() {
	Enttry e;
	return e.run();
}
#	endif
#endif