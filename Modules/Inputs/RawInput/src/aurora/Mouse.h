#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::raw_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		virtual void AE_CALL _rawInput(const RAWINPUT& rawInput) override;
	};
}