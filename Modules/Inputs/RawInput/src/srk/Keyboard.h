#pragma once

#include "DeviceBase.h"

namespace srk::modules::inputs::raw_input {
	class SRK_MODULE_DLL Keyboard : public DeviceBase {
	public:
		Keyboard(Input& input, IWindow& win, const InternalDeviceInfo& info);

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void SRK_CALL poll(bool dispatchEvent) override;

	protected:
		using StateBuffer = uint8_t[256];

		mutable std::shared_mutex _mutex;
		StateBuffer _state;

		mutable std::shared_mutex _listenMutex;
		StateBuffer _listenState;

		virtual void SRK_CALL _rawInput(const RAWINPUT& rawInput) override;

		static int32_t SRK_CALL _getStateIndex(const RAWKEYBOARD& raw);
	};
}