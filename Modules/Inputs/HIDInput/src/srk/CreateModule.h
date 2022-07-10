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

		auto win = (IWindow*)args->tryGet("win").toNumber<uintptr_t>();
		if (!win) {
			printaln(L"HIDInputModule create error : no win"sv);
			return nullptr;
		}

		auto filter = args ? args->tryGet("filter").toEnum<DeviceType>(DeviceType::GAMEPAD) : DeviceType::GAMEPAD;

		return new hid_input::Input(loader, win, filter);
	}
}
#endif