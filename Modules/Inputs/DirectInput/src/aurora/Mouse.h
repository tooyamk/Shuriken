#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual Key::CodeType AE_CALL getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CodeType count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		mutable std::shared_mutex _mutex;
		DIMOUSESTATE2 _state;
		POINT _pos;

		static void AE_CALL _amendmentRelativePos(int32_t& target, LONG absolutePos, LONG referenceRelativePos, int32_t nIndex);
	};
}