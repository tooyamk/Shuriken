#include "Monitor.h"
#include "srk/String.h"

namespace srk {
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

#if SRK_OS == SRK_OS_WINDOWS
		DEVMODEW dm;

		auto wadpName = String::utf8ToWide<std::wstring>(_adapterName);

		uint32_t modeIdx = 0;

		do {
			memset(&dm, 0, sizeof(DEVMODEW));
			dm.dmSize = sizeof(DEVMODEW);

			if (!EnumDisplaySettingsW(wadpName.data(), modeIdx, &dm)) break;

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
				mode._supported = ChangeDisplaySettingsExW(wadpName.data(), &dm, nullptr, CDS_TEST, nullptr) == DISP_CHANGE_SUCCESSFUL;
			} else {
				mode._supported = true;
			}

			//add mode
		} while (true);
#endif
		return std::move(videoModes);
	}

	std::vector<Monitor> Monitor::getMonitors() {
		std::vector<Monitor> monitors;
		
#if SRK_OS == SRK_OS_WINDOWS
		DISPLAY_DEVICEW adapter, display;

		const uint32_t strBufLen = sizeof(display.DeviceString) << 2;
		char strBuf[strBufLen];

		for (uint32_t adapterIdx = 0; ; ++adapterIdx) {
			memset(&adapter, 0, sizeof(DISPLAY_DEVICEW));
			adapter.cb = sizeof(DISPLAY_DEVICEW);

			if (!EnumDisplayDevicesW(nullptr, adapterIdx, &adapter, 0)) break;
			if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;

			bool isPrimary = adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE;
			bool isModesPruned = adapter.StateFlags & DISPLAY_DEVICE_MODESPRUNED;

			for (uint32_t displayIdx = 0; ; ++displayIdx) {
				memset(&display, 0, sizeof(DISPLAY_DEVICEW));
				display.cb = sizeof(DISPLAY_DEVICEW);
				
				bool hasDisplay = EnumDisplayDevicesW(adapter.DeviceName, displayIdx, &display, 0);
				if (hasDisplay || !displayIdx) {
					auto& monitor = monitors.emplace_back();

					monitor._primary = isPrimary;
					monitor._modesPruned = isModesPruned;

					auto strBytes = String::wideToUtf8(std::wstring_view(adapter.DeviceName, sizeof(adapter.DeviceName)), strBuf, strBufLen);
					strBuf[strBytes] = 0;
					monitor._adapterName = (char*)strBuf;

					strBytes = String::wideToUtf8(std::wstring_view(adapter.DeviceString, sizeof(adapter.DeviceString)), strBuf, strBufLen);
					strBuf[strBytes] = 0;
					monitor._adapterDesc = (char*)strBuf;

					if (hasDisplay) {
						strBytes = String::wideToUtf8(std::wstring_view(display.DeviceName, sizeof(display.DeviceName)), strBuf, strBufLen);
						strBuf[strBytes] = 0;
						monitor._deviceName = (char*)strBuf;

						strBytes = String::wideToUtf8(std::wstring_view(display.DeviceString, sizeof(display.DeviceString)), strBuf, strBufLen);
						strBuf[strBytes] = 0;
						monitor._deviceDesc = (char*)strBuf;
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