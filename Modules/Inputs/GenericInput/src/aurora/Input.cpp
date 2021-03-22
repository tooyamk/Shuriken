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

		auto isHID = false;

		printdln("Device ", ": VendorID = ", devDesc.idVendor, "  ProductID = ", devDesc.idProduct, "  NumConfs = ", devDesc.bNumConfigurations, "  bus = ", libusb_get_bus_number(device), "  Device{Cls = ", devDesc.bDeviceClass, "  SubCls = ", devDesc.bDeviceSubClass, "  Protocol = ", devDesc.bDeviceProtocol, "}");

		InternalDeviceInfo best;
		for (size_t i = 0; i < devDesc.bNumConfigurations; ++i) {
			libusb_config_descriptor* conf = nullptr;
			InternalDeviceInfo cur;

			if (libusb_get_config_descriptor(device, i, &conf) == LIBUSB_SUCCESS) {
				printdln("  Configuration ", i, " : NumInterfaces = ", conf->bNumInterfaces, "  ConfValue = ", conf->bConfigurationValue);

				for (decltype(conf->bNumInterfaces) j = 0; j < conf->bNumInterfaces; ++j) {
					auto& interface = conf->interface[j];
					
					for (decltype(interface.num_altsetting) k = 0; k < interface.num_altsetting; ++k) {
						auto& interfaceDesc = interface.altsetting[k];
						if (interfaceDesc.bNumEndpoints && interfaceDesc.bInterfaceClass == libusb_class_code::LIBUSB_CLASS_HID) {
							isHID = true;
							break;

							if (interfaceDesc.bInterfaceSubClass == 1) {
								if (interfaceDesc.bInterfaceProtocol == 1) {
									++cur.score;
								} else if (interfaceDesc.bInterfaceProtocol == 2) {
									++cur.score;
								}
							}

							printdln("    Interface ", j, " ", k, " : Len = ", interfaceDesc.bLength, "  AlternateSetting = ", interfaceDesc.bAlternateSetting, "  NumEps = ", interfaceDesc.bNumEndpoints, "  Interface{Number = ", interfaceDesc.bInterfaceNumber, "  Cls = ", interfaceDesc.bInterfaceClass, "  SubCls = ", interfaceDesc.bInterfaceSubClass, "  Protocol = ", interfaceDesc.bInterfaceProtocol, "}");

							for (size_t l = 0; l < interfaceDesc.bNumEndpoints; ++l) {
								auto& ep = interfaceDesc.endpoint[l];

								printdln("      Endpoint ", l, " : Len = ", ep.bLength, "  EpAddr = ", ep.bEndpointAddress, "  Interval = ", ep.bInterval, "  Attrs = ", ep.bmAttributes, "  Refresh = ", ep.bRefresh, "  MaxPktSize = ", ep.wMaxPacketSize);
							}
						}
					}

					if (isHID) break;
				}

				libusb_free_config_descriptor(conf);
			}

			if (isHID) break;
		}

		if (isHID) {
			std::string_view HID_REPORT_ITEM_TYPE[] = { 
				"main"sv, 
				"global"sv,
				"local"sv 
			};
			std::string_view HID_REPORT_MAIN_ITEM_TAG[] = { 
				"input"sv, 
				"output"sv, 
				"collection"sv, 
				"feature"sv , 
				"end_collection"sv 
			};
			std::string_view HID_REPORT_GLOBAL_ITEM_TAG[] = { 
				"usage_page"sv,
				"logical_minimum"sv, 
				"logical_maximum"sv, 
				"physical_minimum"sv , 
				"physical_maximum"sv, 
				"unit_exponent"sv,
				"unit"sv, 
				"report_size"sv, 
				"report_id"sv, 
				"report_count"sv, 
				"push"sv, 
				"pop"sv 
			};
			std::string_view HID_REPORT_USAGE_PAGE_TYPE[] = {
				"undefined"sv,
				"generic_desktop_page"sv,
				"simulation_controls_page"sv,
				"vr_controls_page"sv ,
				"sport_controls_page"sv,
				"game_controls_page"sv,
				"generic_controls_page"sv,
				"keyboard_or_keypad_page"sv,
				"led_page"sv,
				"button_page"sv,
				"ordinal_page"sv,
				"telephony_device_page"sv,
				"consumer_page"sv,
				"digitizers_page"sv,
				"haptics_page"sv,
				"pid_page"sv ,
				"unicode_page"sv,
				"eye_and_head_trackers_page"sv,
				"auxiliary_display_page"sv,
				"sensors_page"sv,
				"media_instrument_page"sv,
				"braille_display_page"sv,
				"lighting_and_illumination_page"sv,
				"bar_code_scanner_page"sv,
				"scale_page"sv,
				"magnetic_stripe_reding_devices"sv,
				"reserved_point_of_sale_pages"sv,
				"camera_control_page"sv ,
				"arcade_page"sv,
				"ganning_device_page"sv,
				"fido_alliance_page"sv

			};
			std::string_view HID_REPORT_LOCAL_ITEM_TAG[] = { 
				"usage"sv,
				"usage_minimum"sv, 
				"uaage_maximum"sv 
			};
			std::string_view HID_REPORT_COLLECTION_DATA[] = {
				"physical"sv,
				"application"sv,
				"logical"sv,
				"report"sv,
				"named_array"sv,
				"usage_modifier"sv,
				"usage_switch"sv
			};

			//auto nnn = sizeof(Str1);
			uint8_t buf[256];
			if (auto ret = libusb_get_descriptor(handle, libusb_descriptor_type::LIBUSB_DT_HID, 0, buf, sizeof(buf)); ret) {
				printdln();

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

							auto data = ba.slice(size);
							ba.skip(size);

							std::string info = "item(";

							switch ((HIDReportItemType)type) {
							case HIDReportItemType::MAIN:
							{
								info += "main ";
								if (tag >= (uint8_t)HIDReportMainItemTag::BEGIN && tag <= (uint8_t)HIDReportMainItemTag::END) {
									info += HID_REPORT_MAIN_ITEM_TAG[tag - (uint8_t)HIDReportMainItemTag::BEGIN];

									switch ((HIDReportMainItemTag)tag) {
									case HIDReportMainItemTag::COLLECTION:
									{
										if (data.getBytesAvailable()) {
											auto val = data.read<uint8_t>();
											info += " ";
											if (val >= (uint8_t)HIDReportCollectionData::RESERVED_BEGIN && val <= (uint8_t)HIDReportCollectionData::RESERVED_END) {
												info += "reserved";
											} else if (val >= (uint8_t)HIDReportCollectionData::VENDOR_DEFINED_BEGIN && val <= (uint8_t)HIDReportCollectionData::VENDOR_DEFINED_END) {
												info += "vendor_defined";
											} else {
												info += HID_REPORT_COLLECTION_DATA[val];
											}

											info += "  =====";
										}

										break;
									}
									case HIDReportMainItemTag::END_COLLECTION:
									{
										info += "  =====";

										break;
									}
									default:
										break;
									}
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

									switch ((HIDReportGlobalItemTag)tag) {
									case HIDReportGlobalItemTag::USAGE_PAGE:
									{
										if (data.getBytesAvailable() >= 2) {
											auto val = data.read<uint16_t>();
											info += " ";
											if (val == 0x11 || val == 0x13 || (val >= 0x15 && val <= 0x1F) || (val >= 0x21 && val <= 0x3F) ||
												(val >= 0x42 && val <= 0x58) || (val >= 0x5A && val <= 0x7F) || (val >= 0x88 && val <= 0x8B) ||
												(val >= 0x93 && val <= 0xF1CF) || (val >= 0xF1D1 && val <= 0xFEFF)) {
												info += "reserved";
											} else if (val >= (uint16_t)HIDReportUsagePageType::MONITOR_PAGES_BEGIN && val <= (uint16_t)HIDReportUsagePageType::MONITOR_PAGES_END) {
												info += "monitor_pages";
											} else if (val >= (uint16_t)HIDReportUsagePageType::POWER_PAGES_BEGIN && val <= (uint16_t)HIDReportUsagePageType::POWER_PAGES_END) {
												info += "power_pages";
											} else if (val >= (uint16_t)HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && val <= (uint16_t)HIDReportUsagePageType::VENDOR_DEFINED_END) {
												info += "vendor_defined";
											} else {
												info += HID_REPORT_USAGE_PAGE_TYPE[val];
											}

											info += "  =====";
										}

										break;
									}
									default:
										break;
									}
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

							if (data.getBytesAvailable()) {
								info += " ";
								do {
									info += String::toString(data.read<uint8_t>()) + " ";
								} while (data.getBytesAvailable());
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
		}

		libusb_close(handle);
	}
}