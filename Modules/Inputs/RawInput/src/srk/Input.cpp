#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "CreateModule.h"
#include "srk/hash/xxHash.h"

namespace srk::modules::inputs::raw_input {
	Input::Input(Ref* loader, const CreateInputModuleDesc& desc) :
		_loader(loader),
		_win(desc.window),
		_filters(desc.filters),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()),
		_numKeyboards(0),
		_numMouses(0) {
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace std::literals;
		using namespace srk::enum_operators;

		std::vector<InternalDeviceInfo> newDevices;

		constexpr UINT ALLOC_DEV_COUNT = 32;
		RAWINPUTDEVICELIST devices[ALLOC_DEV_COUNT];
		auto deviceCount = ALLOC_DEV_COUNT;
		deviceCount = GetRawInputDeviceList(devices, &deviceCount, sizeof(RAWINPUTDEVICELIST));

		char buffer[256];
		UINT pcbSize = sizeof(buffer);

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

			auto& di = newDevices.emplace_back();
			di.hDevice = dev.hDevice;
			di.type = dt;

			if (auto size = GetRawInputDeviceInfoA(dev.hDevice, RIDI_DEVICENAME, buffer, &pcbSize); size > 0) {
				std::string_view info(buffer, size - 1);

				if (auto p = String::find(info, "VID_"sv); p != std::string_view::npos) di.vendorID = String::toNumber<uint16_t>(info.substr(p + 4, 4), 16);
				if (auto p = String::find(info, "PID_"sv); p != std::string_view::npos) di.productID = String::toNumber<uint16_t>(info.substr(p + 4, 4), 16);

				auto hd = (uintptr_t)dev.hDevice;
				di.guid.set<false, false>(&hd, sizeof(hd));
				
				auto hash = hash::xxHash<64>::calc<std::endian::native>(info.data(), info.size(), 0);
				di.guid.set<false, true>(&hash, sizeof(hash), sizeof(hd));
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
		std::shared_lock lock(_mutex);

		for (auto& info : _devices) {
			if (info.guid == guid && (info.type == DeviceType::KEYBOARD || info.type == DeviceType::MOUSE)) {
				registerRawInputDevices(info.type);

				if (info.type == DeviceType::KEYBOARD) {
					return new Keyboard(*this, *_win, info);
				} else {
					return new Mouse(*this, *_win, info);
				}
			}
		}

		return nullptr;
	}

	HWND Input::getHWND() const {
		return (HWND)_win->getNative(windows::WindowNative::WINDOW);
	}

	void Input::registerRawInputDevices(DeviceType type) {
		if (auto n = _getNumVal(type); n) {
			std::scoped_lock lock(_numMutex);

			if (++(*n) == 1) _registerDevices<false>(type);
		}
	}

	void Input::unregisterRawInputDevices(DeviceType type) {
		if (auto n = _getNumVal(type); n) {
			std::scoped_lock lock(_numMutex);

			if (--(*n) == 0)  _registerDevices<true>(type);
		}
	}
}