#include "Input.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "CreateModule.h"
#include "srk/StringUtility.h"
#include "srk/hash/xxHash.h"
#include <hidsdi.h>

namespace srk::modules::inputs::raw_input {
	Input::Input(Ref* loader, const CreateInputModuleDescriptor& desc) :
		_loader(loader),
		_win(desc.window),
		_filters(desc.filters),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()),
		_numKeyboards(0),
		_numMouses(0) {
		using namespace std::string_view_literals;

		_hwnd = (HWND)_win->getNative("HWND"sv);
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace std::string_view_literals;
		using namespace srk::enum_operators;

		std::vector<InternalDeviceInfo> newDevices;

		constexpr UINT ALLOC_DEV_COUNT = 32;
		RAWINPUTDEVICELIST devices[ALLOC_DEV_COUNT];
		auto deviceCount = ALLOC_DEV_COUNT;
		deviceCount = GetRawInputDeviceList(devices, &deviceCount, sizeof(RAWINPUTDEVICELIST));

		char devNameBuffer[MAX_PATH];
		UINT devNameBufferSize = sizeof(devNameBuffer);

		wchar_t deviceName[256];

		for (decltype(deviceCount) i = 0; i < deviceCount; ++i) {
			auto dev = devices[i];
			
			auto dt = DeviceType::UNKNOWN;
			switch (dev.dwType) {
			case RIM_TYPEKEYBOARD:
				dt = DeviceType::KEYBOARD;
				break;
			case RIM_TYPEMOUSE:
				dt = DeviceType::MOUSE;
				break;
			default:
				break;
			}

			if ((dt & _filters) == DeviceType::UNKNOWN) continue;

			auto& info = newDevices.emplace_back();
			info.hDevice = dev.hDevice;
			info.type = dt;

			if (auto size = GetRawInputDeviceInfoA(dev.hDevice, RIDI_DEVICENAME, devNameBuffer, &devNameBufferSize); size != (UINT)-1) {
				std::string_view devName(devNameBuffer, size - 1);

				if (auto p = devName.find("VID_"sv); p != std::string_view::npos) info.vendorID = StringUtility::toNumber<uint16_t>(devName.substr(p + 4, 4), 16);
				if (auto p = devName.find("PID_"sv); p != std::string_view::npos) info.productID = StringUtility::toNumber<uint16_t>(devName.substr(p + 4, 4), 16);

				auto hd = (uintptr_t)dev.hDevice;
				info.guid.set<false, false>(&hd, sizeof(hd));
				
				auto hash = hash::xxHash<64>::calc(devName.data(), devName.size(), 0);
				info.guid.set<false, true>(&hash, sizeof(hash), sizeof(hd));

				if (auto hidHandle = CreateFileA(devName.data(), 0, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr); hidHandle != INVALID_HANDLE_VALUE) {
					if (HidD_GetProductString(hidHandle, deviceName, sizeof(deviceName))) info.name = StringUtility::wideToUtf8<std::string>(deviceName);
					CloseHandle(hidHandle);
				}
			}
		}

		std::vector<DeviceInfo> add;
		std::vector<DeviceInfo> remove;
		{
			std::scoped_lock lock(_mutex);

			for (auto& info : newDevices) {
				if (!_hasDevice(info, _devices)) add.emplace_back(info);
			}

			for (auto& info : _devices) {
				if (!_hasDevice(info, newDevices)) remove.emplace_back(info);
			}

			_devices = std::move(newDevices);
		}

		for (auto& info : remove) _eventDispatcher->dispatchEvent(this, ModuleEvent::DISCONNECTED, &info);
		for (auto& info : add) _eventDispatcher->dispatchEvent(this, ModuleEvent::CONNECTED, &info);
	}

	IntrusivePtr<IInputDevice> Input::createDevice(const DeviceGUID& guid) {
		InternalDeviceInfo info;
		auto found = false;

		{
			std::shared_lock lock(_mutex);

			for (auto& i : _devices) {
				if (i.guid == guid) {
					info = i;
					found = true;

					break;
				}
			}
		}

		if (!found) return nullptr;

		switch (info.type) {
		case DeviceType::KEYBOARD:
		{
			auto driver = KeyboardDriver::create(*this, *_win, info.hDevice);
			if (driver) return new GenericKeyboard(info, *driver);

			break;
		}
		case DeviceType::MOUSE:
		{
			auto driver = MouseDriver::create(*this, *_win, info.hDevice);
			if (driver) return new GenericMouse(info, *driver);

			break;
		}
		}

		return nullptr;
	}

	void Input::registerRawInputDevices(DeviceType type) {
		if (auto n = _getNumVal(type); n) {
			std::scoped_lock lock(_numMutex);

			if (++(*n) == 1) _registerDevices(type, false);
		}
	}

	void Input::unregisterRawInputDevices(DeviceType type) {
		if (auto n = _getNumVal(type); n) {
			std::scoped_lock lock(_numMutex);

			if (--(*n) == 0)  _registerDevices(type, true);
		}
	}

	uint32_t* Input::_getNumVal(DeviceType type) {
		switch (type) {
		case DeviceType::KEYBOARD:
			return &_numKeyboards;
		case DeviceType::MOUSE:
			return &_numMouses;
		default:
			return nullptr;
		}
	}

	void Input::_registerDevices(DeviceType type, bool remove) {
		RAWINPUTDEVICE dev;
		dev.usUsagePage = 0x1;
		switch (type) {
		case DeviceType::KEYBOARD:
			dev.usUsage = 0x6;
			break;
		case DeviceType::MOUSE:
			dev.usUsage = 0x2;
			break;
		case DeviceType::GAMEPAD:
			dev.usUsage = 0x5;
			break;
		default:
			return;
		}
		dev.dwFlags = remove ? RIDEV_REMOVE : RIDEV_INPUTSINK;
		dev.hwndTarget = getHWND();

		RegisterRawInputDevices(&dev, 1, sizeof(RAWINPUTDEVICE));
	}
}