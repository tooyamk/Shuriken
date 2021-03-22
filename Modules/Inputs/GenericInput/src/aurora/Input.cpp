#include "Input.h"
#include "CreateModule.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::generic_input {
	Input::Input(Ref* loader) :
		_loader(loader),
		_context(nullptr) {
		libusb_init(&_context);
		libusb_set_debug(_context, 3);
	}

	Input::~Input() {
		if (_context) libusb_exit(_context);
	}

	events::IEventDispatcher<ModuleEvent>& Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		if (!_context) return;

		_findDevices();

		//printdln("libusb Version:", LIBUSB_API_VERSION);

		
	}

	IInputDevice* Input::createDevice(const DeviceGUID& guid) {
		return nullptr;
	}

	void Input::_calcGUID(libusb_device* device, const libusb_device_descriptor& desc, DeviceGUID& guid) {
		uint32_t offset = 0;
		guid.set<false, false>(&desc.idVendor, sizeof(desc.idVendor), offset);
		offset += sizeof(desc.idVendor);
		guid.set<false, false>(&desc.idProduct, sizeof(desc.idProduct), offset);
		offset += sizeof(desc.idProduct);
		auto bus = libusb_get_bus_number(device);
		guid.set<false, false>(&bus, sizeof(bus), offset);
		offset += sizeof(bus);

		uint8_t ports[8];
		auto count = libusb_get_port_numbers(device, ports, sizeof(ports));
		if (count < 0) count = 0;

		guid.set<false, true>(ports, count, offset);
	}

	void Input::_findDevices() {
		libusb_device** devices = nullptr;

		auto numDevs = libusb_get_device_list(_context, &devices);

		for (size_t i = 0; i < numDevs; ++i) _checkDevice(devices[i]);

		libusb_free_device_list(devices, 0);

		int a = 1;
	}

	void Input::_checkDevice(libusb_device* device) {
		libusb_device_handle* handle = nullptr;
		
		libusb_device_descriptor desc;
		libusb_get_device_descriptor(device, &desc);

		if (desc.bDeviceClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE || desc.bDeviceSubClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE || desc.bDeviceSubClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE) return;

		DeviceGUID guid(NO_INIT);
		_calcGUID(device, desc, guid);

		if (libusb_open(device, &handle) != LIBUSB_SUCCESS) return;

		printdln("Device ", ": VendorID = ", desc.idVendor, "  ProductID = ", desc.idProduct, "  NumConfs = ", desc.bNumConfigurations, "  bus = ", libusb_get_bus_number(device), "  Device{Cls = ", desc.bDeviceClass, "  SubCls = ", desc.bDeviceSubClass, "  Protocol = ", desc.bDeviceProtocol, "}");

		InternalDeviceInfo best;
		for (size_t i = 0; i < desc.bNumConfigurations; ++i) {
			libusb_config_descriptor* conf = nullptr;
			InternalDeviceInfo cur;

			uint8_t buf[256];
			auto aaa = libusb_get_descriptor(handle, LIBUSB_DT_HID, 1, buf, 256);
			if (libusb_get_config_descriptor(device, i, &conf) == LIBUSB_SUCCESS) {
				printdln("  Configuration ", i, " : NumInterfaces = ", conf->bNumInterfaces, "  ConfValue = ", conf->bConfigurationValue);

				for (decltype(conf->bNumInterfaces) j = 0; j < conf->bNumInterfaces; ++j) {
					auto& interface = conf->interface[j];
					
					for (decltype(interface.num_altsetting) k = 0; k < interface.num_altsetting; ++k) {
						auto& desc = interface.altsetting[k];
						if (desc.bNumEndpoints && desc.bInterfaceClass == libusb_class_code::LIBUSB_CLASS_HID) {
							if (desc.bInterfaceSubClass == 1) {
								if (desc.bInterfaceProtocol == 1) {
									++cur.score;
								} else if (desc.bInterfaceProtocol == 2) {
									++cur.score;
								}
							}

							printdln("    Interface ", j, " ", k, " : Len = ", desc.bLength, "  AlternateSetting = ", desc.bAlternateSetting, "  NumEps = ", desc.bNumEndpoints, "  Interface{Number = ", desc.bInterfaceNumber, "  Cls = ", desc.bInterfaceClass, "  SubCls = ", desc.bInterfaceSubClass, "  Protocol = ", desc.bInterfaceProtocol, "}");

							for (size_t l = 0; l < desc.bNumEndpoints; ++l) {
								auto& ep = desc.endpoint[l];

								printdln("      Endpoint ", l, " : Len = ", ep.bLength, "  EpAddr = ", ep.bEndpointAddress, "  Interval = ", ep.bInterval, "  Attrs = ", ep.bmAttributes, "  Refresh = ", ep.bRefresh, "  MaxPktSize = ", ep.wMaxPacketSize);
							}
						}
					}
				}

				libusb_free_config_descriptor(conf);
			}
		}

		libusb_close(handle);
	}
}