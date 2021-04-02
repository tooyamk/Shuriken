#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::raw_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		using StateBuffer = uint8_t[16];

		mutable std::shared_mutex _mutex;
		StateBuffer _state;
		POINT _pos;

		mutable std::shared_mutex _listenMutex;
		StateBuffer _listenState;
		POINT _listenLastPos;

		virtual void AE_CALL _rawInput(const RAWINPUT& rawInput) override;

		static void AE_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
	};
}