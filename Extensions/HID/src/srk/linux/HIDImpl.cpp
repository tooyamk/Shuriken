#include "linux/HIDImpl.h"

#if SRK_OS == SRK_OS_LINUX
#include "srk/Printer.h"

#include <fcntl.h>
#include <poll.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <libudev.h>

namespace srk::extensions {
	HIDDeviceInfo::HIDDeviceInfo() :
		index(-1),
		vendorID(0),
		productID(0),
		usagePage(0),
		usage(0) {}


	HIDDevice::HIDDevice(int32_t handle) :
		handle(handle) {
	}


	void HID::enumDevices(void* custom, HID::EnumDevicesCallback callback) {
		if (!callback) return;

		auto udev = udev_new();
		auto enumerate = udev_enumerate_new(udev);

		udev_enumerate_add_match_subsystem(enumerate, "hidraw");
		udev_enumerate_scan_devices(enumerate);

		uint8_t reportDesc[HID_MAX_DESCRIPTOR_SIZE];

		for (auto device = udev_enumerate_get_list_entry(enumerate); device; device = udev_list_entry_get_next(device)) {
			uint32_t busType;
			char* serialNumber = nullptr, *productName = nullptr;

			HIDDeviceInfo info;

			auto sysfsPath = udev_list_entry_get_name(device);
			auto rawDev = udev_device_new_from_syspath(udev, sysfsPath);
			auto hidDev = udev_device_get_parent_with_subsystem_devtype(rawDev, "hid", nullptr);
			do {
				if (!hidDev) break;

				auto uevt = udev_device_get_sysattr_value(hidDev, "uevent");
				auto tmp = strdup(uevt);
				char* savePtr = nullptr;
				auto foundId = false, foundSerial = false, foundName = false;
				auto line = strtok_r(tmp, "\n", &savePtr);
				while (line) {
					if (auto value = strchr(line, '='); value) {
						*value = '\0';
						++value;

						if (strcmp(line, "HID_ID") == 0) {
							if (sscanf(value, "%x:%hx:%hx", &busType, &info.vendorID, &info.productID) == 3) foundId = true;
						} else if (strcmp(line, "HID_UNIQ") == 0) {
							serialNumber = strdup(value);
							foundSerial = true;
						} else if (strcmp(line, "HID_NAME") == 0) {
							productName = strdup(value);
							foundName = true;
						}
					}

					line = strtok_r(nullptr, "\n", &savePtr);
				}
				free(tmp);
				if (!foundId || !foundSerial || !foundName) break;

				if (busType != BUS_BLUETOOTH && busType != BUS_I2C && busType != BUS_USB) break;

				if (auto devPath = udev_device_get_devnode(rawDev); devPath) info.pathView = devPath;

				switch (busType) {
				case BUS_USB:
				{
					auto usbDev = udev_device_get_parent_with_subsystem_devtype(rawDev, "usb", "usb_device");
					if (usbDev) {
						info.manufacturer = udev_device_get_sysattr_value(usbDev, "manufacturer");
						info.product = udev_device_get_sysattr_value(usbDev, "product");
					} else {
						info.product = productName;
					}

					break;
				}
				case BUS_BLUETOOTH:
				case BUS_I2C:
					info.product = productName;
					break;
				default:
					break;
				}

				auto rptPathLen = strlen(sysfsPath) + 25 + 1;
				auto rptPath = (char*)calloc(1, rptPathLen);
				snprintf(rptPath, rptPathLen, "%s/device/report_descriptor", sysfsPath);
				
				auto handle = ::open(rptPath, O_RDONLY);
				free(rptPath);
				if (handle < 0) break;

				memset(&reportDesc, 0, sizeof(reportDesc));
				auto reportDescSize = ::read(handle, reportDesc, HID_MAX_DESCRIPTOR_SIZE);
				::close(handle);
				if (reportDescSize < 0) break;

				ByteArray ba(reportDesc, reportDescSize, ByteArray::Usage::SHARED);
				HIDReportDescriptorItem item;
				uint32_t collection = 0;
				int32_t index = 0;
				while (ba.getBytesAvailable()) {
					if (auto n = HIDReportDescriptorItem::parse(ba.getCurrentSource(), ba.getBytesAvailable(), item); n) {
						auto p = ba.getPosition();
						ba.setPosition(p + n);

						switch (item.type) {
						case HIDReportItemType::MAIN:
						{
							switch ((HIDReportMainItemTag)item.tag) {
							case HIDReportMainItemTag::COLLECTION:
								++collection;
								break;
							case HIDReportMainItemTag::END_COLLECTION:
								--collection;
								break;
							default:
								break;
							}
						}
						case HIDReportItemType::GLOBAL:
						{
							if (collection == 0) {
								switch ((HIDReportGlobalItemTag)item.tag) {
								case HIDReportGlobalItemTag::USAGE_PAGE:
									info.usagePage = ba.read<ba_vt::UIX>(item.size);
									break;
								default:
									break;
								}
							}

							break;
						}
						case HIDReportItemType::LOCAL:
						{
							if (collection == 0) {
								switch ((HIDReportLocalItemTag)item.tag) {
								case HIDReportLocalItemTag::USAGE:
								{
									info.usage = ba.read<ba_vt::UIX>(item.size);
									info.index = index++;
									callback(info, custom);
									
									break;
								}
								default:
									break;
								}
							}

							break;
						}
						}

						ba.setPosition(p + n + item.size);
					} else {
						break;
					}
				}
			} while (false);

			free(serialNumber);
			free(productName);
			udev_device_unref(rawDev);
		}

		udev_enumerate_unref(enumerate);
		udev_unref(udev);
	}

	bool HID::isValid(const HIDDeviceInfo& info) {
		return info.index >= 0 ;
	}

	bool HID::isValid(const HIDDevice& device) {
		return false;
	}

	uint16_t HID::getVendorID(const HIDDeviceInfo& info) {
		return info.index >= 0  ? info.vendorID : 0;
	}

	uint16_t HID::getProductID(const HIDDeviceInfo& info) {
		return info.index >= 0  ? info.productID : 0;
	}

	std::string_view HID::getManufacturerString(const HIDDeviceInfo& info) {
		return info.index >= 0  ? info.manufacturer : std::string_view();
	}

	std::string_view HID::getProductString(const HIDDeviceInfo& info) {
		return info.index >= 0  ? info.product : std::string_view();
	}

	std::string_view HID::getPath(const HIDDeviceInfo& info) {
		return info.index >= 0  ? info.pathView : std::string_view();
	}

	uint16_t HID::getUsagePage(const HIDDeviceInfo& info) {
		return info.index >= 0  ? info.usagePage : 0;
	}

	uint16_t HID::getUsage(const HIDDeviceInfo& info) {
		return info.index >= 0 ? info.usage : 0;
	}

	int32_t HID::getIndex(const HIDDeviceInfo& info) {
		return info.index;
	}

	HIDDevice* HID::open(const std::string_view& path) {
		auto handle = ::open(path.data(), O_RDWR | O_NONBLOCK);
		if (handle < 0) return nullptr;

		auto dev = new HIDDevice(handle);

		return dev;
	}

	void HID::close(HIDDevice& device) {
		if (device.handle >= 0) ::close(device.handle);
		delete& device;
	}

	ByteArray HID::getReportDescriptor(const HIDDevice& device) {
		if (device.handle < 0) return ByteArray();

		hidraw_report_descriptor rptDesc;
		if (ioctl(device.handle, HIDIOCGRDESCSIZE, &rptDesc.size) < 0) return ByteArray();
		if (ioctl(device.handle, HIDIOCGRDESC, &rptDesc) < 0) return ByteArray();

		ByteArray ba(rptDesc.size);
		ba.write<ba_vt::BYTE>(rptDesc.value, rptDesc.size);
		ba.seekBegin();

		return ba;
	}

	size_t HID::read(HIDDevice& device, void* data, size_t dataLength, size_t timeout) {
		do {
			if (timeout) {
				pollfd fds;

				fds.fd = device.handle;
				fds.events = POLLIN;
				fds.revents = 0;
				if (auto ret = poll(&fds, 1, timeout); ret == 0) {
					if (timeout == HID::IN_TIMEOUT_BLOCKING) {
						continue;
					} else {
						return HID::OUT_WAITTING;
					}
				} else if (ret == -1) {
					return HID::OUT_ERROR;
				} else {
					if (fds.revents & (POLLERR | POLLHUP | POLLNVAL)) return HID::OUT_ERROR;
				}
			}

			auto bytesReaded = ::read(device.handle, data, dataLength);
			if (bytesReaded < 0) {
				if (errno == EAGAIN || errno == EINPROGRESS) {
					if (timeout == HID::IN_TIMEOUT_BLOCKING) {
						continue;
					} else {
						return HID::OUT_WAITTING;
					}
				}

				return HID::OUT_ERROR;
			}

			return bytesReaded;
		} while (true);
	}

	size_t HID::write(HIDDevice& device, const void* data, size_t dataLength, size_t timeout) {
		do {
			if (timeout) {
				pollfd fds;

				fds.fd = device.handle;
				fds.events = POLLOUT;
				fds.revents = 0;
				if (auto ret = poll(&fds, 1, timeout); ret == 0) {
					if (timeout == HID::IN_TIMEOUT_BLOCKING) {
						continue;
					} else {
						return HID::OUT_WAITTING;
					}
				} else if (ret == -1) {
					return HID::OUT_ERROR;
				} else {
					if (fds.revents & (POLLERR | POLLHUP | POLLNVAL)) return HID::OUT_ERROR;
				}
			}

			auto bytesReaded = ::write(device.handle, data, dataLength);
			if (bytesReaded < 0) {
				if (errno == EAGAIN || errno == EINPROGRESS) {
					if (timeout == HID::IN_TIMEOUT_BLOCKING) {
						continue;
					} else {
						return HID::OUT_WAITTING;
					}
				}

				return HID::OUT_ERROR;
			}

			return bytesReaded;
		} while (true);

		return HID::OUT_ERROR;
	}
}
#endif