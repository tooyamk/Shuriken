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
#include <android/native_activity.h>
#	include <dlfcn.h>
#include <limits.h>

    #include <stdlib.h>

    #include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

void onStart(ANativeActivity* activity) {
    using namespace std::string_view_literals;

	printaln(L"onStart"sv);
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
    using namespace std::string_view_literals;

    printaln(L"onNativeWindowCreated"sv);
}

void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    using namespace std::string_view_literals;

    printaln(L"onNativeWindowCreated"sv);
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
    using namespace std::string_view_literals;

	printaln(L"ANativeActivity_onCreate 2"sv);
    printaln(L"internalDataPath : "sv, std::string_view(activity->internalDataPath));
    printaln(L"externalDataPath : "sv, std::string_view(activity->externalDataPath));
    //mac _NSGetExecutablePath() (man 3 dyld)

    printaln(L"std::filesystem:current_path : "sv, std::filesystem::current_path().wstring());

    {
        /*std::filesystem::path dir(activity->externalDataPath);
        printaln(L"for"sv);
        for (auto& itr : std::filesystem::directory_iterator(dir)) {
            printaln(L"sub : "sv, itr.path().wstring());
        }*/
    }

    auto clazz = activity->env->GetObjectClass(activity->clazz);
    auto methodID = activity->env->GetMethodID(clazz, "getPackageCodePath", "()Ljava/lang/String;");
    auto result = activity->env->CallObjectMethod(activity->clazz, methodID);
    jboolean isCopy;
    auto m_pStrBuf = activity->env->GetStringUTFChars((jstring)result, &isCopy);
    auto m_nStrLen = activity->env->GetStringLength((jstring)result);
    std::filesystem::path dir(std::string_view(m_pStrBuf, m_nStrLen));
    auto appDir = dir.parent_path().u8string();

    printaln(L"jni : "sv, appDir);

    printaln(L"for"sv);
    for (auto& itr : std::filesystem::directory_iterator(std::filesystem::path("/data/data/com.shuriken.test"))) {
        printaln(L"sub : "sv, itr.path().wstring());
    }

    activity->env->ReleaseStringUTFChars((jstring)result, m_pStrBuf);

    {
        char path1[PATH_MAX];
        auto cmd = "/proc/" + String::toString(getpid()) + "/cmdline";
        //auto cmd = "/proc/self/cmdline";
        if (realpath(cmd.data(), path1)) {
            printaln(cmd, L" : "sv, true, L"   ="sv,  std::string_view(path1));

            auto f = fopen(cmd.data(), "r");
            printaln(L"fopen : "sv, f != nullptr);
            if (f) {
                char buffer[PATH_MAX];
                memset(buffer, 0, PATH_MAX);
                fgets(buffer, sizeof(buffer), f);
                printaln(L"fopen buf : "sv, std::string_view(buffer));
                fclose(f);
            }

            f = fopen("/proc/self/maps", "r");
            printaln(L"fopen : "sv, f != nullptr);
            if (f) {
                char buffer[PATH_MAX];
                memset(buffer, 0, PATH_MAX);
                fgets(buffer, sizeof(buffer), f);
                printaln(L"fopen buf : "sv, std::string_view(buffer));
                fclose(f);

                uint64_t low, high;
                char perms[5] = { 0 };
                uint64_t offset;
                uint32_t major, minor;
                char path[PATH_MAX] = { 0 };
                uint32_t inode;
                auto rst = sscanf(buffer, "%" PRIx64 "-%" PRIx64 " %s %" PRIx64 " %x:%x %u [%s\n", &low, &high, perms, &offset, &major, &minor, &inode, path);
                printaln(L"sscanf low="sv, low, L" high="sv, high, L" perms="sv, perms, L" offset="sv, offset, L" major="sv, major, L" minor="sv, minor, L" inode="sv, inode, L" path="sv, path, L" rst="sv, rst);
                if (realpath(path, buffer)) {
                    printaln(L"realpath : "sv, std::string_view(buffer));
                }
            }
        } else {
            printaln(cmd, L" : "sv, false, L"   ="sv);
        }
    }
    
    {
        char path2[PATH_MAX];
        if (realpath("/proc/self/exe", path2)) {
            printaln(L"exe : "sv, true, L"   ="sv,  std::string_view(path2));
        } else {
            printaln(L"exe : "sv, false, L"   ="sv);
        }

        printaln(L"getAppPath : "sv, Application::getAppPath().wstring());
    }

    {
        DynamicLibraryLoader loader;
        auto path = getDllPath("zstd");
        auto b = loader.load(path);
        printaln(L"load zstd : "sv, path, "   ", b);
    }

    {
        uint8_t buffer[10];
        uint8_t* buf = buffer;
        ++buf;
        ++buf;
        printaln(L"try write float32_t"sv);
        *((float32_t*)buf) = 0.1f;
        printaln(L"writed float32_t"sv);
        printaln(L"try read float32_t"sv);
        auto f = ((float32_t*)buf)[0];
        printaln(L"readed float32_t  "sv, f, L"   "sv, &f);
    }

    bindLifeCycle(activity);
}
#	else
int32_t main() {
	Enttry e;
	return e.run();
}
#	endif
#endif