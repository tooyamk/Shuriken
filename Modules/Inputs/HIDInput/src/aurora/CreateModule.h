#pragma once

#include "Input.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::inputs {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		if (!args) {
			printdln(L"HIDInputModule create error : no args"sv);
			return nullptr;
		}

		auto app = (IApplication*)args->tryGet("app").toNumber<uintptr_t>();
		if (!app) {
			printdln(L"HIDInputModule create error : no app"sv);
			return nullptr;
		}

		auto filter = args ? args->tryGet("filter").toEnum<DeviceType>(DeviceType::GAMEPAD) : DeviceType::GAMEPAD;

		return new hid_input::Input(loader, app, filter);
	}
}
#endif