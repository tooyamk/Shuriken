#pragma once

#include "Input.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace srk::enum_operators;

		auto filter = args ? args->tryGet("filter").toEnum<DeviceType>(DeviceType::GAMEPAD) : DeviceType::GAMEPAD;

		return new xinput::Input(loader, filter);
	}
}
#endif