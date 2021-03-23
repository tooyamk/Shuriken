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

		uint8_t manufacturer[256];
		libusb_get_string_descriptor_ascii(handle, devDesc.iManufacturer, manufacturer, sizeof(manufacturer));

		uint8_t product[256];
		libusb_get_string_descriptor_ascii(handle, devDesc.iProduct, product, sizeof(product));

		auto isHID = false;

		printdln("Device ", ": VendorID = ", devDesc.idVendor, "  ProductID = ", devDesc.idProduct, "  [", (const char*)manufacturer, "]  [", (const char*)product, "]  NumConfs = ", devDesc.bNumConfigurations, "  bus = ", libusb_get_bus_number(device), "  Device{Cls = ", devDesc.bDeviceClass, "  SubCls = ", devDesc.bDeviceSubClass, "  Protocol = ", devDesc.bDeviceProtocol, "}");

		InternalDeviceInfo best;
		for (size_t i = 0; i < devDesc.bNumConfigurations; ++i) {
			libusb_config_descriptor* conf = nullptr;
			InternalDeviceInfo cur;

			if (libusb_get_config_descriptor(device, i, &conf) == LIBUSB_SUCCESS) {
				//printdln("  Configuration ", i, " : NumInterfaces = ", conf->bNumInterfaces, "  ConfValue = ", conf->bConfigurationValue);

				for (decltype(conf->bNumInterfaces) j = 0; j < conf->bNumInterfaces; ++j) {
					auto& interface = conf->interface[j];
					
					for (decltype(interface.num_altsetting) k = 0; k < interface.num_altsetting; ++k) {
						auto& interfaceDesc = interface.altsetting[k];
						if (interfaceDesc.bNumEndpoints && interfaceDesc.bInterfaceClass == libusb_class_code::LIBUSB_CLASS_HID) {
							isHID = true;
							//break;

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

					//if (isHID) break;
				}

				libusb_free_config_descriptor(conf);
			}

			//if (isHID) break;
		}

		if (isHID) {
			//auto nnn = sizeof(Str1);
			uint8_t buf[256];
			if (auto ret = libusb_get_descriptor(handle, libusb_descriptor_type::LIBUSB_DT_HID, 0, buf, sizeof(buf)); ret) {
				HIDDescriptor hidDesc;
				hidDesc.set(buf, ret);

				if (hidDesc.bNumDescriptors > 0 && hidDesc.DescriptorList[0].bType == libusb_descriptor_type::LIBUSB_DT_REPORT) {
					if (auto ret = libusb_get_descriptor(handle, libusb_descriptor_type::LIBUSB_DT_REPORT, 0, buf, sizeof(buf)); ret) {
						std::string indent = "";
						auto isFirstNotCollectionMain = true;
						HIDReportUsagePageType usagePage = HIDReportUsagePageType::UNDEFINED;

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

							std::string info = indent + "item("sv;

							if (auto itr = HID_REPORT_ITEM_TYPE_MAP.find((HIDReportItemType)type); itr != HID_REPORT_ITEM_TYPE_MAP.end()) {
								info += itr->second;
								info += " ";

								switch ((HIDReportItemType)type) {
								case HIDReportItemType::MAIN:
								{
									if (auto itr = HID_REPORT_MAIN_ITEM_TAG_MAP.find((HIDReportMainItemTag)tag); itr != HID_REPORT_MAIN_ITEM_TAG_MAP.end()) {
										info += itr->second;

										switch ((HIDReportMainItemTag)tag) {
										case HIDReportMainItemTag::COLLECTION:
										{
											indent += "  "sv;

											if (data.getBytesAvailable()) {
												auto val = data.read<uint8_t>();
												info += " ";
												if (val >= (uint16_t)HIDReportCollectionData::VENDOR_DEFINED_BEGIN && val <= (uint16_t)HIDReportCollectionData::VENDOR_DEFINED_END) {
													info += "VENDOR_DEFINED"sv;
												} else {
													if (auto itr = HID_REPORT_COLLECTION_DATA_MAP.find((HIDReportCollectionData)val); itr != HID_REPORT_COLLECTION_DATA_MAP.end()) {
														info += itr->second;
													} else {
														info += "RESERVED"sv;
													}
												}
											}

											break;
										}
										case HIDReportMainItemTag::END_COLLECTION:
										{
											auto n = isFirstNotCollectionMain ? 2 : 4;
											indent = indent.substr(0, indent.size() - n);
											info = info.substr(n);

											break;
										}
										default:
										{
											if (isFirstNotCollectionMain) {
												isFirstNotCollectionMain = false;

												indent += "  "sv;
											} else {
												info = info.substr(2);
											}

											if (data.getBytesAvailable()) {
												auto val = data.read<uint8_t>();

												info += " ["sv;

												info += val >> 0 & 0b1 ? "Constant "sv : "Data "sv;
												info += val >> 1 & 0b1 ? "Variable "sv : "Array "sv;
												info += val >> 2 & 0b1 ? "Relative "sv : "Absolute "sv;
												info += val >> 3 & 0b1 ? "Wrap "sv : "NoWrap "sv;
												info += val >> 4 & 0b1 ? "NonLinear "sv : "Linear "sv;
												info += val >> 5 & 0b1 ? "NoPreferred "sv : "PreferredState "sv;
												info += val >> 6 & 0b1 ? "NullState "sv : "NoNullPosition "sv;
												info += val >> 7 & 0b1 ? "Volatile"sv : "NonVolatile"sv;

												if (data.getBytesAvailable()) {
													val = data.read<uint8_t>();
													info += val >> 0 & 0b1 ? " BufferedBytes"sv : " BitField"sv;
												}

												info += "]"sv;
											}

											break;
										}
										}
									} else {
										info += String::toString(tag);
									}


									break;
								}
								case HIDReportItemType::GLOBAL:
								{
									if (auto itr = HID_REPORT_GLOBAL_ITEM_TAG_MAP.find((HIDReportGlobalItemTag)tag); itr != HID_REPORT_GLOBAL_ITEM_TAG_MAP.end()) {
										info += itr->second;

										switch ((HIDReportGlobalItemTag)tag) {
										case HIDReportGlobalItemTag::USAGE_PAGE:
										{
											if (data.getBytesAvailable()) {
												usagePage = (HIDReportUsagePageType)data.read<ba_vt::UIX>(data.getBytesAvailable());
												info += " ";

												if (usagePage >= HIDReportUsagePageType::POWER_PAGES_BEGIN && usagePage <= HIDReportUsagePageType::POWER_PAGES_END) {
													info += "POWER_PAGES"sv;
												} else if (usagePage >= HIDReportUsagePageType::VENDOR_DEFINED_BEGIN && usagePage <= HIDReportUsagePageType::VENDOR_DEFINED_END) {
													info += "VENDOR_DEFINED"sv;
												} else {
													if (auto itr = HID_REPORT_USAGE_PAGE_TYPE_MAP.find(usagePage); itr != HID_REPORT_USAGE_PAGE_TYPE_MAP.end()) {
														info += itr->second;
													} else {
														info += "RESERVED"sv;
													}
												}
											}

											break;
										}
										case HIDReportGlobalItemTag::REPORT_SIZE:
										{
											info += " "sv + String::toString(data.read<ba_vt::UIX>(data.getBytesAvailable())) + "bits"sv;
											data.seekEnd();

											break;
										}
										case HIDReportGlobalItemTag::REPORT_COUNT:
										case HIDReportGlobalItemTag::LOGICAL_MINIMUM:
										case HIDReportGlobalItemTag::LOGICAL_MAXIMUM:
										{
											info += " " + String::toString(data.read<ba_vt::UIX>(data.getBytesAvailable()));
											data.seekEnd();

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
									if (auto itr = HID_REPORT_LOCAL_ITEM_TAG_MAP.find((HIDReportLocalItemTag)tag); itr != HID_REPORT_LOCAL_ITEM_TAG_MAP.end()) {
										info += itr->second;

										switch (usagePage) {
										case HIDReportUsagePageType::GENERIC_DESKTOP:
										{
											if (data.getBytesAvailable()) {
												info += " ";
												auto val = (HIDReportGenericDesktopPageType)data.read<ba_vt::UIX>(data.getBytesAvailable());
												if (auto itr = HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_GENERIC_DISKTOP_PAGE_TYPE_MAP.end()) {
													info += itr->second;
												} else {
													info += "RESERVED"sv;
												}
											}

											break;
										}
										case HIDReportUsagePageType::CONSUMER_DEVICES:
										{
											if (data.getBytesAvailable()) {
												info += " ";
												auto val = (HIDReportConsumerPageType)data.read<ba_vt::UIX>(data.getBytesAvailable());
												if (auto itr = HID_REPORT_CONSUMER_PAGE_TYPE_MAP.find(val); itr != HID_REPORT_CONSUMER_PAGE_TYPE_MAP.end()) {
													info += itr->second;
												} else {
													info += "RESERVED"sv;
												}
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
								default:
									break;
								}
							} else {
								info += String::toString(type);
							}

							info += " "sv + String::toString(size) + ")"sv;

							if (data.getBytesAvailable()) {
								data.seekBegin();
								info += " "sv;
								do {
									info += String::toString(data.read<uint8_t>()) + " "sv;
								} while (data.getBytesAvailable());
							} else {
								info += "  ====="sv;
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

		printdln();
	}
}