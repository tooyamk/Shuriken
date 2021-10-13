#pragma once

#include "Input.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::inputs {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;
		using namespace aurora::enum_operators;

		if (!args) {
			printaln(L"DirectInputModule create error : no args"sv);
			return nullptr;
		}

		auto app = (IApplication*)args->tryGet("app").toNumber<uintptr_t>();
		if (!app) {
			printaln(L"DirectInputModule create error : no app"sv);
			return nullptr;
		}

		auto filter = args->tryGet("filter").toEnum<DeviceType>(DeviceType::KEYBOARD | DeviceType::MOUSE | DeviceType::GAMEPAD);
		auto ignoreXInputDevices = args->tryGet("ignoreXInputDevices").toBool();

		return new direct_input::Input(loader, app, filter, ignoreXInputDevices);
	}
}
#endif