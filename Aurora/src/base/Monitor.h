#pragma once

#include "base/LowLevel.h"
#include <string>
#include <vector>

namespace aurora {
	class AE_DLL Monitor {
	public:
		class AE_DLL VideoMode {
		public:
		private:
			bool _supported = false;
			i32 _bitsPerPixel = 0;
			i32 _horizontalPixels = 0;
			i32 _verticalPixels = 0;
			i32 _refreshRate = 0;//hz

			friend Monitor;
		};


		Monitor();

		inline bool isPrimary() const {
			return _primary;
		}

		inline bool isModesPruned() const {
			return _modesPruned;
		}

		inline const std::string& getDeviceName() const {
			return _deviceName;
		}

		inline const std::string& getDeviceDescription() const {
			return _deviceDesc;
		}

		inline const std::string& getAdapterName() const {
			return _adapterName;
		}

		inline const std::string& getAdapterDescription() const {
			return _adapterDesc;
		}

		inline i32 AE_CALL gethorizontalSize() const {
			return _horizontalSize;
		}

		inline i32 AE_CALL getVerticalSize() const {
			return _verticalSize;
		}

		inline i32 AE_CALL gethorizontalPixels() const {
			return _horizontalPixels;
		}

		inline i32 AE_CALL getVerticalPixels() const {
			return _verticalPixels;
		}

		std::vector<VideoMode> AE_CALL getVideoModes() const;

		static std::vector<Monitor> AE_CALL getMonitors();

	private:
		bool _primary;
		bool _modesPruned;//The device has more display modes than its output devices support.

		std::string _deviceName;
		std::string _deviceDesc;
		std::string _adapterName;
		std::string _adapterDesc;

		i32 _horizontalSize;//physics size mm
		i32 _verticalSize;//physics size mm
		i32 _horizontalPixels;
		i32 _verticalPixels;
	};
}