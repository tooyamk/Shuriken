#pragma once

#include "Input.h"
#include "srk/Debug.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		if (!args) {
			printaln(L"HIDInputModule create error : no args"sv);
			return nullptr;
		}

		auto app = (IApplication*)args->tryGet("app").toNumber<uintptr_t>();
		if (!app) {
			printaln(L"HIDInputModule create error : no app"sv);
			return nullptr;
		}

		auto filter = args ? args->tryGet("filter").toEnum<DeviceType>(DeviceType::GAMEPAD) : DeviceType::GAMEPAD;

		return new hid_input::Input(loader, app, filter);
	}
}
#endif