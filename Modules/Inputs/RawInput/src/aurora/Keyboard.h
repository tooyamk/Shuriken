#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::raw_input {
	class AE_MODULE_DLL Keyboard : public DeviceBase {
	public:
		Keyboard(Input& input, IApplication& app, const InternalDeviceInfo& info);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		using StateBuffer = uint8_t[256];

		mutable std::shared_mutex _mutex;
		StateBuffer _state;

		mutable std::shared_mutex _listenMutex;
		StateBuffer _listenState;

		virtual void AE_CALL _rawInput(const RAWINPUT& rawInput) override;

		static int32_t AE_CALL _getStateIndex(const RAWKEYBOARD& raw);
	};
}