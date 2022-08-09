#pragma once

#include "Input.h"
#include "srk/Debug.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;
		using namespace srk::enum_operators;

		if (!args) {
			printaln(L"DirectInputModule create error : no args"sv);
			return nullptr;
		}

		auto app = (windows::IWindow*)args->tryGet("win").toNumber<uintptr_t>();
		if (!app) {
			printaln(L"DirectInputModule create error : no win"sv);
			return nullptr;
		}

		auto filter = args->tryGet("filter").toEnum<DeviceType>(DeviceType::KEYBOARD | DeviceType::MOUSE | DeviceType::GAMEPAD);
		auto ignoreXInputDevices = args->tryGet("ignoreXInputDevices").toBool();

		return new direct_input::Input(loader, app, filter, ignoreXInputDevices);
	}
}
#endif