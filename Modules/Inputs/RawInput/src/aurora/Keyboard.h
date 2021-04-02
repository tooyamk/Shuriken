#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::raw_input {
	class AE_MODULE_DLL Keyboard : public DeviceBase {
	public:
		Keyboard(Input& input, IApplication& app, const InternalDeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;
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