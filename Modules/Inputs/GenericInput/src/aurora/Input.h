#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::generic_input {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const DeviceGUID& guid) override;

	private:
		struct InternalDeviceInfo {
			int32_t readInterfaceIdx = -1, readEndpointIdx = -1, writeInterfaceIdx = -1, writeEndpointIdx = -1;
			DeviceType bestType = DeviceType::UNKNOWN;
			size_t score = 0;
		};


		IntrusivePtr<Ref> _loader;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		std::vector<DeviceInfo> _devices;
		std::vector<DeviceInfo> _newDevices;
		std::vector<uint32_t> _keepDevices;

		libusb_context* _context;

		void AE_CALL _calcGUID(libusb_device* device, const libusb_device_descriptor& desc, DeviceGUID& guid);
		void AE_CALL _findDevices();
		void AE_CALL _checkDevice(libusb_device* device);
	};
}