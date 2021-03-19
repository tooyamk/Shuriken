#include "Input.h"
#include "CreateModule.h"

namespace aurora::modules::inputs::generic_input {
	Input::Input(Ref* loader, IApplication* app) :
		_loader(loader),
		_app(app) {
	}

	Input::~Input() {
	}

	events::IEventDispatcher<ModuleEvent>& Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
	}

	IInputDevice* Input::createDevice(const DeviceGUID& guid) {
		/*
		printf("libusb Version:%d\n", LIBUSB_API_VERSION);

		libusb_device** devs = nullptr;
		libusb_context* context = nullptr;
		if (!libusb_init(&context)) {
			auto numDevs = libusb_get_device_list(context, &devs);
			for (size_t i = 0; i < numDevs; ++i) {
				auto dev = devs[i];

				libusb_device_handle* handle = nullptr;
				//if (!libusb_open(dev, &handle)) {
				libusb_device_descriptor desc;
				libusb_get_device_descriptor(dev, &desc);

				printf("Device %d: VendorID = %d  ProductID = %d  NumConfs = %d  Device{Cls = %d  SubCls = %d  Protocol = %d}\n", i, desc.idVendor, desc.idProduct, desc.bNumConfigurations, desc.bDeviceClass, desc.bDeviceSubClass, desc.bDeviceProtocol);

				for (size_t j = 0; j < desc.bNumConfigurations; ++j) {
					libusb_config_descriptor* conf = nullptr;
					if (!libusb_get_config_descriptor(dev, j, &conf)) {
						printf("  Configuration %d : NumInterfaces = %d  ConfValue = %d\n", j, conf->bNumInterfaces, conf->bConfigurationValue);

						for (size_t k = 0; k < conf->bNumInterfaces; ++k) {
							auto& interface = conf->interface[k];

							for (size_t l = 0; l < interface.num_altsetting; ++l) {
								auto& desc = interface.altsetting[l];
								printf("    Interface %d %d : Len = %d  AlternateSetting = %d  NumEps = %d  Interface{Number = %d  Cls = %d  SubCls = %d  Protocol = %d}\n", k, l, desc.bLength, desc.bAlternateSetting, desc.bNumEndpoints, desc.bInterfaceNumber, desc.bInterfaceClass, desc.bInterfaceSubClass, desc.bInterfaceProtocol);

								for (size_t m = 0; m < desc.bNumEndpoints; ++m) {
									auto& ep = desc.endpoint[m];

									printf("      Endpoint %d : Len = %d  EpAddr = %d  Interval = %d  Attrs = %d  Refresh = %d  MaxPktSize = %d\n", m, ep.bLength, ep.bEndpointAddress, ep.bInterval, ep.bmAttributes, ep.bRefresh, ep.wMaxPacketSize);
								}
							}
						}

						libusb_free_config_descriptor(conf);
					}
				}

				//libusb_close(handle);
			//}
			}

			libusb_free_device_list(devs, 0);
			libusb_exit(context);
		}
		*/
		return nullptr;
	}
}