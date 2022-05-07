#pragma once

#include "DeviceBase.h"

namespace srk::modules::inputs::raw_input {
	class SRK_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info);

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void SRK_CALL poll(bool dispatchEvent) override;

	protected:
		using StateBuffer = uint8_t[16];

		union Point {
			uint64_t combined;
			struct {
				int32_t x, y;
			};
		};

		mutable std::shared_mutex _mutex;
		StateBuffer _state;
		std::atomic_uint64_t _pos;

		mutable std::shared_mutex _listenMutex;
		StateBuffer _listenState;
		std::atomic_uint64_t _listenLastPos;
		std::atomic_int32_t _lastWheel;

		virtual void SRK_CALL _rawInput(const RAWINPUT& rawInput) override;

		static void SRK_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
		static Point SRK_CALL _getCursorPos();
	};
}