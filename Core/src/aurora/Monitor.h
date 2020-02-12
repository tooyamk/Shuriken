#pragma once

#include "aurora/Global.h"
#include <vector>

namespace aurora {
	class AE_DLL Monitor {
	public:
		class AE_DLL VideoMode {
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

		inline int32_t AE_CALL gethorizontalSize() const {
			return _horizontalSize;
		}

		inline int32_t AE_CALL getVerticalSize() const {
			return _verticalSize;
		}

		inline int32_t AE_CALL gethorizontalPixels() const {
			return _horizontalPixels;
		}

		inline int32_t AE_CALL getVerticalPixels() const {
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

		int32_t _horizontalSize;//physics size mm
		int32_t _verticalSize;//physics size mm
		int32_t _horizontalPixels;
		int32_t _verticalPixels;
	};
}