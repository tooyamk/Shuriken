#pragma once

#include "Input.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::inputs {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace aurora::enum_operators;

		auto filter = args ? args->tryGet("filter").toEnum<DeviceType>(DeviceType::GAMEPAD) : DeviceType::GAMEPAD;

		return new xinput::Input(loader, filter);
	}
}
#endif