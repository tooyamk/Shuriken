#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		mutable std::shared_mutex _mutex;
		DIMOUSESTATE2 _state;
		POINT _pos;

		POINT AE_CALL _getClientPos() const;

		static void AE_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
	};
}