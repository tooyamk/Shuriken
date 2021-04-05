#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::raw_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info);

		virtual Key::CountType AE_CALL getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		using StateBuffer = uint8_t[16];

		mutable std::shared_mutex _mutex;
		StateBuffer _state;
		POINT _pos;

		mutable std::shared_mutex _listenMutex;
		StateBuffer _listenState;
		std::atomic_uint64_t _listenLastPos;
		std::atomic_int32_t _lastWheel;

		virtual void AE_CALL _rawInput(const RAWINPUT& rawInput) override;

		static void AE_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
	};
}