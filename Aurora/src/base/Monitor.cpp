#include "Monitor.h"
#include "base/String.h"

namespace aurora {
	Monitor::Monitor() :
		_primary(false),
		_modesPruned(false),
		_horizontalSize(0),
		_verticalSize(0),
		_horizontalPixels(0),
		_verticalPixels(0) {
	}

	std::vector<Monitor::VideoMode> Monitor::getVideoModes() const {
		std::vector<VideoMode> videoModes;

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		DEVMODEW dm;

		wchar_t* wadpName = nullptr;
		String::Utf8ToUnicode(_adapterName.c_str(), 0xFFFFFFFFui32, wadpName);

		ui32 modeIdx = 0;

		do {
			memset(&dm, 0, sizeof(DEVMODEW));
			dm.dmSize = sizeof(DEVMODEW);

			if (!EnumDisplaySettingsW(wadpName, modeIdx, &dm)) break;

			++modeIdx;

			bool same = false;
			for (auto& m : videoModes) {
				if (m._bitsPerPixel == dm.dmBitsPerPel &&
					m._horizontalPixels == dm.dmPelsWidth &&
					m._verticalPixels == dm.dmPelsHeight &&
					m._refreshRate == dm.dmDisplayFrequency) {
					same = true;
					break;
				}
			}
			if (same) continue;

			auto& mode = videoModes.emplace_back();
			mode._bitsPerPixel = dm.dmBitsPerPel;
			mode._horizontalPixels = dm.dmPelsWidth;
			mode._verticalPixels = dm.dmPelsHeight;
			mode._refreshRate = dm.dmDisplayFrequency;

			// Skip modes with less than 15 BPP
			//if (dm.dmBitsPerPel < 15) continue;

			if (_modesPruned) {
				// Skip modes not supported by the connected displays
				mode._supported = ChangeDisplaySettingsExW(wadpName, &dm, nullptr, CDS_TEST, nullptr) == DISP_CHANGE_SUCCESSFUL;
			} else {
				mode._supported = true;
			}

			//add mode
		} while (true);

		delete[] wadpName;
#endif
		return std::move(videoModes);
	}

	std::vector<Monitor> Monitor::getMonitors() {
		std::vector<Monitor> monitors;
		
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		DISPLAY_DEVICEW adapter, display;

		const ui32 strBufLen = sizeof(display.DeviceString) << 2;
		i8 strBuf[strBufLen];

		for (ui32 adapterIdx = 0; ; ++adapterIdx) {
			memset(&adapter, 0, sizeof(DISPLAY_DEVICEW));
			adapter.cb = sizeof(DISPLAY_DEVICEW);

			if (!EnumDisplayDevicesW(nullptr, adapterIdx, &adapter, 0)) break;
			if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;

			bool isPrimary = adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE;
			bool isModesPruned = adapter.StateFlags & DISPLAY_DEVICE_MODESPRUNED;

			for (ui32 displayIdx = 0; ; ++displayIdx) {
				memset(&display, 0, sizeof(DISPLAY_DEVICEW));
				display.cb = sizeof(DISPLAY_DEVICEW);
				
				bool hasDisplay = EnumDisplayDevicesW(adapter.DeviceName, displayIdx, &display, 0);
				if (hasDisplay || !displayIdx) {
					auto& monitor = monitors.emplace_back();

					monitor._primary = isPrimary;
					monitor._modesPruned = isModesPruned;

					auto strLen = String::UnicodeToUtf8(adapter.DeviceName, sizeof(adapter.DeviceName), strBuf, strBufLen);
					strBuf[strLen] = 0;
					monitor._adapterName = strBuf;

					strLen = String::UnicodeToUtf8(adapter.DeviceString, sizeof(adapter.DeviceString), strBuf, strBufLen);
					strBuf[strLen] = 0;
					monitor._adapterDesc = strBuf;

					if (hasDisplay) {
						strLen = String::UnicodeToUtf8(display.DeviceName, sizeof(display.DeviceName), strBuf, strBufLen);
						strBuf[strLen] = 0;
						monitor._deviceName = strBuf;

						strLen = String::UnicodeToUtf8(display.DeviceString, sizeof(display.DeviceString), strBuf, strBufLen);
						strBuf[strLen] = 0;
						monitor._deviceDesc = strBuf;
					}

					HDC dc = CreateDCW(L"DISPLAY", adapter.DeviceName, nullptr, nullptr);

					monitor._horizontalSize = GetDeviceCaps(dc, HORZSIZE);
					monitor._verticalSize = GetDeviceCaps(dc, VERTSIZE);
					monitor._horizontalPixels = GetDeviceCaps(dc, HORZRES);
					monitor._verticalPixels = GetDeviceCaps(dc, VERTRES);

					DeleteDC(dc);
				} else {
					break;
				}
			}
		}
#endif
		return std::move(monitors);
	}
}