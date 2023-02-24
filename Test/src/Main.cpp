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
    printaln("onNativeWindowCreated");
}

void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    printaln("onNativeWindowCreated");
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
	printaln("ANativeActivity_onCreate 2");
    printaln("internalDataPath : ", activity->internalDataPath);
    printaln("externalDataPath : ", activity->externalDataPath);
    //mac _NSGetExecutablePath() (man 3 dyld)

    printaln("std::filesystem:current_path : ", std::filesystem::current_path().wstring());

    {
        /*std::filesystem::path dir(activity->externalDataPath);
        printaln("for");
        for (auto& itr : std::filesystem::directory_iterator(dir)) {
            printaln("sub : ", itr.path().wstring());
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

    printaln("jni : ", appDir);

    printaln("for");
    for (auto& itr : std::filesystem::directory_iterator(std::filesystem::path("/data/data/com.shuriken.test"))) {
        printaln("sub : ", itr.path().wstring());
    }

    activity->env->ReleaseStringUTFChars((jstring)result, m_pStrBuf);

    {
        char path1[PATH_MAX];
        auto cmd = "/proc/" + String::toString(getpid()) + "/cmdline";
        //auto cmd = "/proc/self/cmdline";
        if (realpath(cmd.data(), path1)) {
            printaln(cmd, " : ", true, "   =",  std::string_view(path1));

            auto f = fopen(cmd.data(), "r");
            printaln("fopen : ", f != nullptr);
            if (f) {
                char buffer[PATH_MAX];
                memset(buffer, 0, PATH_MAX);
                fgets(buffer, sizeof(buffer), f);
                printaln("fopen buf : ", buffer);
                fclose(f);
            }

            f = fopen("/proc/self/maps", "r");
            printaln("fopen : ", f != nullptr);
            if (f) {
                char buffer[PATH_MAX];
                memset(buffer, 0, PATH_MAX);
                fgets(buffer, sizeof(buffer), f);
                printaln("fopen buf : ", buffer);
                fclose(f);

                uint64_t low, high;
                char perms[5] = { 0 };
                uint64_t offset;
                uint32_t major, minor;
                char path[PATH_MAX] = { 0 };
                uint32_t inode;
                auto rst = sscanf(buffer, "%" PRIx64 "-%" PRIx64 " %s %" PRIx64 " %x:%x %u [%s\n", &low, &high, perms, &offset, &major, &minor, &inode, path);
                printaln("sscanf low=", low, " high=", high, " perms=", perms, " offset=", offset, " major=", major, " minor=", minor, " inode=", inode, " path=", path, " rst=", rst);
                if (realpath(path, buffer)) {
                    printaln("realpath : ", buffer);
                }
            }
        } else {
            printaln(cmd, " : ", false, "   =");
        }
    }
    
    {
        char path2[PATH_MAX];
        if (realpath("/proc/self/exe", path2)) {
            printaln("exe : ", true, "   =",  std::string_view(path2));
        } else {
            printaln("exe : ", false, "   =");
        }

        printaln("getAppPath : ", Application::getAppPath().wstring());
    }

    {
        DynamicLibraryLoader loader;
        auto path = getDllPath("zstd");
        auto b = loader.load(path);
        printaln("load zstd : ", path, "   ", b);
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