#pragma once

#include "srk/Framework.h"
#include <string>
#include <vector>

namespace srk {
	class SRK_FW_DLL Monitor {
	public:
		class SRK_FW_DLL VideoMode {
		public:
		private:
			bool _supported = false;
			int32_t _bitsPerPixel = 0;
			int32_t _horizontalPixels = 0;
			int32_t _verticalPixels = 0;
			int32_t _refreshRate = 0;//hz

			friend Monitor;
		};


		Monitor();

		inline bool SRK_CALL isPrimary() const {
			return _primary;
		}

		inline bool SRK_CALL isModesPruned() const {
			return _modesPruned;
		}

		inline const std::string& SRK_CALL getDeviceName() const {
			return _deviceName;
		}

		inline const std::string& SRK_CALL getDeviceDescription() const {
			return _deviceDesc;
		}

		inline const std::string& SRK_CALL getAdapterName() const {
			return _adapterName;
		}

		inline const std::string& SRK_CALL getAdapterDescription() const {
			return _adapterDesc;
		}

		inline int32_t SRK_CALL gethorizontalSize() const {
			return _horizontalSize;
		}

		inline int32_t SRK_CALL getVerticalSize() const {
			return _verticalSize;
		}

		inline int32_t SRK_CALL gethorizontalPixels() const {
			return _horizontalPixels;
		}

		inline int32_t SRK_CALL getVerticalPixels() const {
			return _verticalPixels;
		}

		std::vector<VideoMode> SRK_CALL getVideoModes() const;

		static std::vector<Monitor> SRK_CALL getMonitors();

	private:
		bool _primary;
		bool _modesPruned;//The device has more display modes than its output devices support.

		std::string _deviceName;
		std::string _deviceDesc;
		std::string _adapterName;
		std::string _adapterDesc;

		int32_t _horizontalSize;//physics size mm
		int32_t _verticalSize;//physics size mm
		int32_t _horizontalPixels;
		int32_t _verticalPixels;
	};
}