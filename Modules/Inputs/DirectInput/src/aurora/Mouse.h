#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		union Point {
			uint64_t combined;
			struct {
				int32_t x, y;
			};
		};

		mutable std::shared_mutex _mutex;
		DIMOUSESTATE2 _state;
		std::atomic_uint64_t _pos;

		static void AE_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
		static Point AE_CALL _getCursorPos();
	};
}