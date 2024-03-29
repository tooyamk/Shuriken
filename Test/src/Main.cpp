#if __has_include("mimalloc-new-delete.h")
#	include "mimalloc-new-delete.h"
#endif
#include "Entry.h"

#if SRK_OS != SRK_OS_MACOS
#	if SRK_OS == SRK_OS_WINDOWS
//#pragma comment(linker, "/subsystem:console")
int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int32_t nCmdShow) {
	SetDllDirectoryW((Application::getAppPath().parent_path().wstring() + L"/libs/").data());
	return Enttry().run();
}
#	elif SRK_OS == SRK_OS_ANDROID
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
#		ifdef SRK_HAS_ANDROID_NATIVE_APPLICATION_H
	extensions::AndroidNativeApplication::init(activity, nullptr, false);
	std::thread([](){ Enttry().run(); }).detach();
#		endif
}
#	else
int32_t main() {
	return Enttry().run();
}
#	endif
#endif