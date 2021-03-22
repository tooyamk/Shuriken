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
		using namespace std::literals;

		libusb_device_handle* handle = nullptr;
		
		libusb_device_descriptor devDesc;
		libusb_get_device_descriptor(device, &devDesc);

		if (devDesc.bDeviceClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE || devDesc.bDeviceSubClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE || devDesc.bDeviceSubClass != libusb_class_code::LIBUSB_CLASS_PER_INTERFACE) return;

		DeviceGUID guid(NO_INIT);
		_calcGUID(device, devDesc, guid);

		if (libusb_open(device, &handle) != LIBUSB_SUCCESS) return;

		printdln("Device ", ": VendorID = ", devDesc.idVendor, "  ProductID = ", devDesc.idProduct, "  NumConfs = ", devDesc.bNumConfigurations, "  bus = ", libusb_get_bus_number(device), "  Device{Cls = ", devDesc.bDeviceClass, "  SubCls = ", devDesc.bDeviceSubClass, "  Protocol = ", devDesc.bDeviceProtocol, "}");

		std::string_view HID_REPORT_ITEM_TYPE[] = {"main"sv, "global"sv, "local"sv};
		std::string_view HID_REPORT_MAIN_ITEM_TAG[] = { "input"sv, "output"sv, "collection"sv, "feature"sv , "end_collection"sv };
		std::string_view HID_REPORT_GLOBAL_ITEM_TAG[] = { "usage_page"sv, "logical_minimum"sv, "logical_maximum"sv, "physical_minimum"sv , "physical_maximum"sv, "unit_exponent"sv, "unit"sv, "report_size"sv, "report_id"sv, "report_count"sv };
		std::string_view HID_REPORT_LOCAL_ITEM_TAG[] = { "usage"sv, "usage_minimum"sv, "uaage_maximum"sv };

		InternalDeviceInfo best;
		for (size_t i = 0; i < devDesc.bNumConfigurations; ++i) {
			libusb_config_descriptor* conf = nullptr;
			InternalDeviceInfo cur;

			uint8_t buf[256];
			auto aaa = libusb_get_descriptor(handle, LIBUSB_DT_HID, 1, buf, 256);
			if (libusb_get_config_descriptor(device, i, &conf) == LIBUSB_SUCCESS) {
				printdln("  Configuration ", i, " : NumInterfaces = ", conf->bNumInterfaces, "  ConfValue = ", conf->bConfigurationValue);

				for (decltype(conf->bNumInterfaces) j = 0; j < conf->bNumInterfaces; ++j) {
					auto& interface = conf->interface[j];
					
					for (decltype(interface.num_altsetting) k = 0; k < interface.num_altsetting; ++k) {
						auto& interfaceDesc = interface.altsetting[k];
						if (interfaceDesc.bNumEndpoints && interfaceDesc.bInterfaceClass == libusb_class_code::LIBUSB_CLASS_HID) {
							if (interfaceDesc.bInterfaceSubClass == 1) {
								if (interfaceDesc.bInterfaceProtocol == 1) {
									++cur.score;
								} else if (interfaceDesc.bInterfaceProtocol == 2) {
									++cur.score;
								}
							}

							//auto nnn = sizeof(Str1);
							uint8_t buf[256];
							if (auto ret = libusb_get_descriptor(handle, libusb_descriptor_type::LIBUSB_DT_HID, 0, buf, sizeof(buf)); ret) {
								HIDDescriptor hidDesc;
								hidDesc.set(buf, ret);

								if (hidDesc.bNumDescriptors > 0 && hidDesc.DescriptorList[0].bType == libusb_descriptor_type::LIBUSB_DT_REPORT) {
									if (auto ret = libusb_get_descriptor(handle, libusb_descriptor_type::LIBUSB_DT_REPORT, 0, buf, sizeof(buf)); ret) {
										ByteArray ba(buf, ret, ByteArray::Usage::SHARED);
										while (ba.getBytesAvailable()) {
											uint8_t item = ba.read<uint8_t>();
											auto size = item & 0b11;
											auto type = item >> 2 & 0b11;
											auto tag = item >> 4 & 0b1111;

											if (type == 3 && tag == 0xF) {
												size = ba.read<uint8_t>();
												tag = ba.read<uint8_t>();
											}

											std::string info = "item(";

											switch ((HIDReportItemType)type) {
											case HIDReportItemType::MAIN:
											{
												info += "main ";
												if (tag >= (uint8_t)HIDReportMainItemTag::BEGIN && tag <= (uint8_t)HIDReportMainItemTag::END) {
													info += HID_REPORT_MAIN_ITEM_TAG[tag - (uint8_t)HIDReportMainItemTag::BEGIN];
												} else {
													info += String::toString(tag);
												}


												break;
											}
											case HIDReportItemType::GLOBAL:
											{
												info += "global ";
												if (tag >= (uint8_t)HIDReportGlobalItemTag::BEGIN && tag <= (uint8_t)HIDReportGlobalItemTag::END) {
													info += HID_REPORT_GLOBAL_ITEM_TAG[tag - (uint8_t)HIDReportGlobalItemTag::BEGIN];
												} else {
													info += String::toString(tag);
												}

												break;
											}
											case HIDReportItemType::LOCAL:
											{
												info += "local ";
												if (tag >= (uint8_t)HIDReportLocalItemTag::BEGIN && tag <= (uint8_t)HIDReportLocalItemTag::END) {
													info += HID_REPORT_LOCAL_ITEM_TAG[tag - (uint8_t)HIDReportLocalItemTag::BEGIN];
												} else {
													info += String::toString(tag);
												}

												break;
											}
											default:
												info += "unknown";
												break;
											}

											info += " " + String::toString(size) + ")";

											if (size) {
												info += " ";
												for (size_t i = 0; i < size; ++i) {
													info += String::toString(ba.read<uint8_t>()) + " ";
												}
											}

											printdln(info);

											//printdln((type == 0 ? ""sv : (type == 1 ? "  "sv : "    "sv)), "type = ", HID_REPORT_ITEM_TYPE[type], "  tag = ", (tag < 8 || tag > 12 ? String::toString(tag) : HID_REPORT_MAIN_ITEM_TAG[tag - 8]));

											//ba.skip(size);
										}

										int a = 1;
									}

									int a = 1;
								}

								int a = 1;
							}


							printdln("    Interface ", j, " ", k, " : Len = ", interfaceDesc.bLength, "  AlternateSetting = ", interfaceDesc.bAlternateSetting, "  NumEps = ", interfaceDesc.bNumEndpoints, "  Interface{Number = ", interfaceDesc.bInterfaceNumber, "  Cls = ", interfaceDesc.bInterfaceClass, "  SubCls = ", interfaceDesc.bInterfaceSubClass, "  Protocol = ", interfaceDesc.bInterfaceProtocol, "}");

							for (size_t l = 0; l < interfaceDesc.bNumEndpoints; ++l) {
								auto& ep = interfaceDesc.endpoint[l];

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