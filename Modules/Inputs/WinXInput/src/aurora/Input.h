#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::win_xinput {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader);
		virtual ~Input();

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const DeviceGUID& guid) override;

	protected:
		virtual RefPtr<Ref> AE_CALL _destruction() const override { return _loader; }

	private:
		RefPtr<Ref> _loader;

		std::vector<DeviceInfo> _devices;
		std::vector<DeviceInfo> _connectedDevices;
		std::vector<uint32_t> _keepDevices;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;
	};
}