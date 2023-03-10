#include "linux/HIDImpl.h"

#if SRK_OS == SRK_OS_LINUX
#include "srk/Debug.h"

#include <fcntl.h>
#include <poll.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <libudev.h>

namespace srk::extensions {
	HIDDeviceInfo::HIDDeviceInfo() :
		valid(false),
		vendorID(0),
		productID(0),
		usagePage(0),
		usage(0) {}


	HIDDevice::HIDDevice(int32_t handle) :
		handle(handle) {}


	int32_t HIDUtils::parseUeventInfo(const char* uevent, uint32_t* busType, uint16_t* vid, uint16_t* pid, char** serialNumber, char** productName) {
		auto tmp = strdup(uevent);
		char* savePtr = nullptr;
		bool foundId = false, foundSerial = false, foundName = false;

		auto line = strtok_r(tmp, "\n", &savePtr);
		while (line != nullptr) {
			auto value = strchr(line, '=');
			if (!value) goto next_line;

			*value = '\0';
			++value;

			if (strcmp(line, "HID_ID") == 0) {
				if (sscanf(value, "%x:%hx:%hx", busType, vid, pid) == 3) foundId = true;
			} else if (strcmp(line, "HID_UNIQ") == 0) {
				*serialNumber = strdup(value);
				foundSerial = true;
			} else if (strcmp(line, "HID_NAME") == 0) {
				*productName = strdup(value);
				foundName = true;
			}

		next_line:
			line = strtok_r(nullptr, "\n", &savePtr);
		}

		free(tmp);

		return foundId && foundSerial && foundName;
	}

	int32_t HIDUtils::getHidReportDescriptor(const char* rptPath, hidraw_report_descriptor* desc) {
		auto handle = ::open(rptPath, O_RDONLY);
		if (handle < 0) return -1;

		memset(desc, 0x0, sizeof(*desc));
		auto res = read(handle, desc->value, HID_MAX_DESCRIPTOR_SIZE);
		desc->size = res;

		close(handle);
		return res;
	}

	int32_t HIDUtils::getHidReportDescriptorFromSysfs(const char* sysfsPath, hidraw_report_descriptor* desc) {
		auto rptPathLen = strlen(sysfsPath) + 25 + 1;
		auto rptPath = (char*)calloc(1, rptPathLen);
		snprintf(rptPath, rptPathLen, "%s/device/report_descriptor", sysfsPath);

		auto res = getHidReportDescriptor(rptPath, desc);
		free(rptPath);

		return res;
	}

	int32_t HIDUtils::getNextHidUsage(uint8_t* reportDesc, uint32_t size, uint32_t* pos, HIDUsagePage& usagePage, HIDUsage& usage) {
		int32_t dataLen, keySize;
		int32_t initial = *pos == 0; /* Used to handle case where no top-level application collection is defined */
		auto usagePairReady = false;

		/* Usage is a Local Item, it must be set before each Main Item (Collection) before a pair is returned */
		auto usageFound = false;

		while (*pos < size) {
			int32_t key = reportDesc[*pos];
			int32_t cmd = key & 0xfc;

			/* Determine data_len and key_size */
			if (!getHidItemSize(reportDesc, *pos, size, &dataLen, &keySize)) return -1; /* malformed report */

			switch (cmd) {
			case 0x4: /* Usage Page 6.2.2.7 (Global) */
				usagePage = HIDUtils::getHidReportBytes(reportDesc, size, dataLen, *pos);
				break;
			case 0x8: /* Usage 6.2.2.8 (Local) */
			{
				usage = HIDUtils::getHidReportBytes(reportDesc, size, dataLen, *pos);
				usageFound = true;

				break;
			}
			case 0xa0: /* Collection 6.2.2.4 (Main) */
			{
				/* A Usage Item (Local) must be found for the pair to be valid */
				if (usageFound) usagePairReady = true;

				/* Usage is a Local Item, unset it */
				usageFound = false;

				break;
			}
			case 0x80: /* Input 6.2.2.4 (Main) */
			case 0x90: /* Output 6.2.2.4 (Main) */
			case 0xB0: /* Feature 6.2.2.4 (Main) */
			case 0xC0: /* End Collection 6.2.2.4 (Main) */
				/* Usage is a Local Item, unset it */
				usageFound = false;
				break;
			}

			/* Skip over this key and it's associated data */
			*pos += dataLen + keySize;

			/* Return usage pair */
			if (usagePairReady) return 0;
		}

		/* If no top-level application collection is found and usage page/usage pair is found, pair is valid
		https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/top-level-collections */
		if (initial && usageFound) return 0; /* success */

		return 1; /* finished processing */
	}

	int32_t HIDUtils::getHidItemSize(uint8_t* reportDesc, uint32_t pos, uint32_t size, int32_t* dataLen, int32_t* keySize) {
		int32_t key = reportDesc[pos];

		/*
		* This is a Long Item. The next byte contains the
		* length of the data section (value) for this key.
		* See the HID specification, version 1.11, section
		* 6.2.2.3, titled "Long Items."
		*/
		if ((key & 0xF0) == 0xF0) {
			if (pos + 1 < size) {
				*dataLen = reportDesc[pos + 1];
				*keySize = 3;
				return 1;
			}
			*dataLen = 0; /* malformed report */
			*keySize = 0;
		}

		/*
		* This is a Short Item. The bottom two bits of the
		* key contain the size code for the data section
		* (value) for this key. Refer to the HID
		* specification, version 1.11, section 6.2.2.2,
		* titled "Short Items."
		*/
		int32_t sizeCode = key & 0x3;
		switch (sizeCode) {
		case 0:
		case 1:
		case 2:
			*dataLen = sizeCode;
			*keySize = 1;
			return 1;
		case 3:
			*dataLen = 4;
			*keySize = 1;
			return 1;
		default:
			/* Can't ever happen since size_code is & 0x3 */
			*dataLen = 0;
			*keySize = 0;
			break;
		};

		/* malformed report */
		return 0;
	}

	uint32_t HIDUtils::getHidReportBytes(uint8_t* reportDesc, size_t len, size_t numBytes, size_t cur) {
		/* Return if there aren't enough bytes. */
		if (cur + numBytes >= len) return 0;

		if (numBytes == 0)
			return 0;
		else if (numBytes == 1)
			return reportDesc[cur + 1];
		else if (numBytes == 2)
			return (reportDesc[cur + 2] * 256 + reportDesc[cur + 1]);
		else if (numBytes == 4)
			return (
				reportDesc[cur + 4] * 0x01000000 +
				reportDesc[cur + 3] * 0x00010000 +
				reportDesc[cur + 2] * 0x00000100 +
				reportDesc[cur + 1] * 0x00000001
				);
		else
			return 0;
	}


	void HID::enumDevices(void* custom, HID::EnumDevicesCallback callback) {
		if (!callback) return;

		auto udev = udev_new();
		auto enumerate = udev_enumerate_new(udev);

		udev_enumerate_add_match_subsystem(enumerate, "hidraw");
		udev_enumerate_scan_devices(enumerate);

		for (auto device = udev_enumerate_get_list_entry(enumerate); device; device = udev_list_entry_get_next(device)) {
			uint32_t busType;
			char* serialNumber = nullptr, * productName = nullptr;
			//const char* devPath = nullptr;
			hidraw_report_descriptor reportDesc;

			HIDDeviceInfo info;

			auto sysfsPath = udev_list_entry_get_name(device);
			auto rawDev = udev_device_new_from_syspath(udev, sysfsPath);
			auto hidDev = udev_device_get_parent_with_subsystem_devtype(rawDev, "hid", nullptr);
			if (!hidDev) goto next;

			if (!HIDUtils::parseUeventInfo(udev_device_get_sysattr_value(hidDev, "uevent"), &busType, &info.vendorID, &info.productID, &serialNumber, &productName)) goto next;

			switch (busType) {
			case BUS_BLUETOOTH:
			case BUS_I2C:
			case BUS_USB:
				break;
			default:
				goto next;
			}

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

			if (HIDUtils::getHidReportDescriptorFromSysfs(sysfsPath, &reportDesc) >= 0) {
				uint32_t pos = 0;

				if (!HIDUtils::getNextHidUsage(reportDesc.value, reportDesc.size, &pos, info.usagePage, info.usage)) {
					info.valid = true;
					callback(info, custom);
				}

				while (!HIDUtils::getNextHidUsage(reportDesc.value, reportDesc.size, &pos, info.usagePage, info.usage)) {
					info.valid = true;
					callback(info, custom);
				}
			}

		next:
			free(serialNumber);
			free(productName);
			udev_device_unref(rawDev);
		}

		udev_enumerate_unref(enumerate);
		udev_unref(udev);
	}

	bool HID::isValid(const HIDDeviceInfo& info) {
		return info.valid;
	}

	bool HID::isValid(const HIDDevice& device) {
		return false;
	}

	uint16_t HID::getVendorID(const HIDDeviceInfo& info) {
		return info.valid ? info.vendorID : 0;
	}

	uint16_t HID::getProductID(const HIDDeviceInfo& info) {
		return info.valid ? info.productID : 0;
	}

	std::string_view HID::getManufacturerString(const HIDDeviceInfo& info) {
		return info.valid ? info.manufacturer : std::string_view();
	}

	std::string_view HID::getProductString(const HIDDeviceInfo& info) {
		return info.valid ? info.product : std::string_view();
	}

	std::string_view HID::getPath(const HIDDeviceInfo& info) {
		return info.valid ? info.pathView : std::string_view();
	}

	uint16_t HID::getUsagePage(const HIDDeviceInfo& info) {
		return info.valid ? info.usagePage : 0;
	}

	uint16_t HID::getUsage(const HIDDeviceInfo& info) {
		return info.valid ? info.usage : 0;
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

	ByteArray HID::getRawReportDescriptor(const HIDDevice& device) {
		if (device.handle < 0) return ByteArray();

		hidraw_report_descriptor rptDesc;
		if (ioctl(device.handle, HIDIOCGRDESCSIZE, &rptDesc.size) < 0) return ByteArray();
		if (ioctl(device.handle, HIDIOCGRDESC, &rptDesc) < 0) return ByteArray();

		ByteArray ba(rptDesc.size);
		ba.write<ba_vt::BYTE>(rptDesc.value, rptDesc.size);
		ba.seekBegin();

		return ba;
	}

	void* HID::getPreparsedData(const HIDDevice& device) {
		return nullptr;
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