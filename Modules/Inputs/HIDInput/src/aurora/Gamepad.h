#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public GamepadBase<64, 64, 64> {
	public:
		Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;

	protected:
		struct InputReportItem {
			aurora::extensions::HIDReportGenericDesktopPageType type = extensions::HIDReportGenericDesktopPageType::UNDEFINED;
			Vec2<int32_t> logical;
			Vec2<int32_t> physical;
			uint32_t size = 0;
			uint32_t count = 0;
		};


		mutable std::shared_mutex _inputStateMutex;

		std::vector<InputReportItem> _inputReportItems;

		virtual void AE_CALL _doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) override;
		virtual bool AE_CALL _doOutput() override;

		void AE_CALL _parseRawReportDescriptor(ByteArray& raw);
		void AE_CALL _parseRawReportItem(aurora::extensions::HIDReportScopeValues& scopeValues, aurora::extensions::HIDReportMainItemTag tag, int32_t val);
		void AE_CALL _parseRawReportItem(aurora::extensions::HIDReportScopeValues& scopeValues, aurora::extensions::HIDReportGlobalItemTag tag, int32_t val);
		void AE_CALL _parseRawReportItem(aurora::extensions::HIDReportScopeValues& scopeValues, aurora::extensions::HIDReportLocalItemTag tag, int32_t val);
	};
}