#pragma once

#include "DeviceBase.h"

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void SRK_CALL poll(bool dispatchEvent) override;

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

		static void SRK_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
		static Point SRK_CALL _getCursorPos();
	};
}