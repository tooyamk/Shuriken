#include "Entry.h"

#if SRK_OS != SRK_OS_MACOS
#	if SRK_OS == SRK_OS_WINDOWS
//#pragma comment(linker, "/subsystem:console")
int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int32_t nCmdShow) {
	SetDllDirectoryW((getAppPath().parent_path().wstring() + L"/libs/").data());

	Enttry e;
	return e.run();
}
#	elif SRK_OS == SRK_OS_ANDROID
#include <android/native_activity.h>

void onStart(ANativeActivity* activity) {
	printaln("onStart");
}

void onResume(ANativeActivity* activity) {

}

void* onSaveInstanceState(ANativeActivity* activity, size_t *outSize) {
  return nullptr;
}

void onPause(ANativeActivity* activity) {

}

void onStop(ANativeActivity* activity) {

}

void onDestroy(ANativeActivity* activity) {

}

void onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {

}

void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {

}

void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {

}

void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    //isLoop = true;
    //activity->instance = (void*) queue;
    //pthread_create(&loopID, NULL, looper, activity);
}

void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    //isLoop = false;
}

void onConfigurationChanged(ANativeActivity* activity) {

}

void onLowMemory(ANativeActivity* activity) {

}

void bindLifeCycle(ANativeActivity* activity) {
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
	printaln("ANativeActivity_onCreate");
    bindLifeCycle(activity);
}
#	else
int32_t main() {
	Enttry e;
	return e.run();
}
#	endif
#endif